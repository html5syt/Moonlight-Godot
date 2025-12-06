#!/usr/bin/env python3
import re
import sys
import os
import argparse
from typing import Tuple, Dict, Optional

# === 全局存储解析结果（仅用于 structs/enums/defines 查询）===
parsed_data = {}


def is_output_param(param_name: str, param_type: str) -> bool:
    """启发式判断是否为输出参数"""
    if '*' not in param_type and '[' not in param_type:
        return False
    lower = param_name.lower()
    return any(kw in lower for kw in ['out', 'buffer', 'data', 'name', 'desc', 'addr'])


def parse_header(filepath: str):
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        lines = f.readlines()

    code = ''
    for line in lines:
        line = line.rstrip()
        if line.endswith('\\'):
            code += line[:-1]
        else:
            code += line + '\n'

    defines = {}
    enums = {}
    structs = {}
    functions = []
    callbacks = []

    # 1. #define
    define_pat = re.compile(r'#define\s+(\w+)\s+([^\n]+)')
    for m in define_pat.finditer(code):
        name, val = m.groups()
        val = val.strip()
        if val.startswith('"') and val.endswith('"'):
            defines[name] = val
        elif re.match(r'^-?\d+$|0x[0-9a-fA-F]+$', val):
            defines[name] = val
        elif val in defines:
            defines[name] = defines[val]

    # 2. enum
    enum_pat = re.compile(r'typedef\s+enum\s*{([^}]*)}\s*(\w+);', re.DOTALL)
    for m in enum_pat.finditer(code):
        body, name = m.groups()
        items = []
        val = 0
        for item in body.split(','):
            item = item.strip()
            if not item or item.startswith('//'):
                continue
            if '=' in item:
                k, v = item.split('=', 1)
                k, v = k.strip(), v.strip()
                items.append((k, v))
                try:
                    val = int(v, 0) + 1
                except:
                    val = f"({v}) + 1"
            else:
                items.append((item, str(val)))
                val += 1
        enums[name] = items

    # 3. struct
    struct_pat = re.compile(r'typedef\s+struct\s+_\w*\s*{([^}]*)}\s*(\w+)', re.DOTALL)
    for m in struct_pat.finditer(code):
        body, name = m.groups()
        fields = []
        for line in body.split('\n'):
            line = line.strip()
            if not line or line.startswith('//'):
                continue
            line = re.sub(r'\s*//.*$', '', line)
            fm = re.match(r'(\w+(?:\s*\*)?)\s+(\w+)(\[[^\]]+\])?;', line)
            if fm:
                ftype, fname, arr = fm.groups()
                ftype = ftype.strip()
                if arr:
                    ftype += arr  # e.g., "char[256]"
                fields.append((ftype, fname))
        structs[name] = fields

    # 4. callback
    cb_pat = re.compile(r'typedef\s+(\w+)\s*\(\*([A-Za-z_]\w*)\)\s*\(([^)]*)\);')
    for m in cb_pat.finditer(code):
        ret, name, params_str = m.groups()
        params = []
        if params_str.strip():
            for p in params_str.split(','):
                p = p.strip()
                if p == 'void':
                    continue
                parts = p.rsplit(' ', 1)
                if len(parts) == 2:
                    ptype, pname = parts
                    params.append((ptype.strip(), pname.strip()))
                else:
                    params.append((p.strip(), f"arg{len(params)}"))
        callbacks.append((name, ret, params))

    # 5. functions (Li*)
    func_pat = re.compile(r'^(\w[\w\s\*]*)\s+(Li\w+)\s*\(([^)]*)\)\s*;', re.MULTILINE)
    for m in func_pat.finditer(code):
        ret, name, params_str = m.groups()
        params = []
        if params_str.strip() and params_str != 'void':
            for p in params_str.split(','):
                p = p.strip()
                if p.endswith('...'):  # variadic
                    continue
                match = re.match(r'^(.+?)\s+(\w+)(\[[^\]]*\])?$', p)
                if match:
                    ptype, pname, arr = match.groups()
                    ptype = ptype.strip()
                    if arr:
                        ptype += arr
                    params.append((ptype, pname))
                else:
                    params.append((p, f"arg{len(params)}"))
        functions.append((name, ret.strip(), params))

    return {
        'defines': defines,
        'enums': enums,
        'structs': structs,
        'functions': functions,
        'callbacks': callbacks,
    }


def godot_type(c_type: str, struct_map: Optional[Dict[str, str]] = None, prefix: str = "") -> str:
    struct_map = struct_map or {}
    c_type = c_type.strip().replace('const ', '')
    base_type = c_type

    # 处理数组和指针的基础类型
    if '[' in c_type:
        base_type = c_type.split('[')[0].strip()
    elif c_type.endswith('*'):
        base_type = c_type[:-1].strip()

    # 去掉可能残留的 const 和 *
    base_type = re.sub(r'const|\*', '', base_type).strip()

    # 数组类型
    if '[' in c_type:
        if 'char' in base_type:
            return 'String'
        elif any(t in base_type for t in ['int', 'short', 'long']):
            return 'PackedInt32Array'
        elif 'float' in base_type or base_type == 'double':
            return 'PackedFloat64Array'
        else:
            return 'PackedByteArray'

    # 指针类型
    if c_type.endswith('*'):
        if 'char' in base_type:
            return 'String'
        elif base_type in struct_map:
            return f"Ref<{struct_map[base_type]}>"
        elif base_type in parsed_data.get('structs', {}):
            return f"Ref<{prefix}{base_type}>"
        else:
            return 'PackedByteArray'

    # 值类型：结构体 → 统一用 Ref（Godot 要求）
    if base_type in struct_map:
        return f"Ref<{struct_map[base_type]}>"
    elif base_type in parsed_data.get('structs', {}):
        return f"Ref<{prefix}{base_type}>"

    # 枚举 → int
    if base_type in parsed_data.get('enums', {}):
        return 'int'

    # 基础类型
    if base_type in ('int', 'short', 'char', 'uint32_t', 'unsigned int', 'long'):
        return 'int'
    elif base_type == 'bool':
        return 'bool'
    elif 'float' in base_type or base_type == 'double':
        return 'double'
    else:
        return 'Variant'


def variant_type_to_property_info(gd_type: str) -> str:
    if gd_type == 'int':
        return "Variant::INT"
    elif gd_type == 'bool':
        return "Variant::BOOL"
    elif gd_type == 'double':
        return "Variant::FLOAT"
    elif gd_type == 'String':
        return "Variant::STRING"
    elif gd_type.startswith('Ref<') or gd_type == 'Variant':
        return "Variant::OBJECT"
    elif 'PackedInt32Array' in gd_type:
        return "Variant::PACKED_INT32_ARRAY"
    elif 'PackedFloat64Array' in gd_type:
        return "Variant::PACKED_FLOAT64_ARRAY"
    elif 'PackedByteArray' in gd_type:
        return "Variant::PACKED_BYTE_ARRAY"
    else:
        return "Variant::OBJECT"


def generate_header(module_name: str, data, prefix: str) -> str:
    godot_class_name = prefix + module_name.capitalize()
    h = f"""#ifndef {module_name.upper()}_H
#define {module_name.upper()}_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/packed_float64_array.hpp>
#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

"""

    # Structs — 加上前缀
    for name, fields in data['structs'].items():
        prefixed_name = prefix + name
        h += f"\nclass {prefixed_name} : public RefCounted {{\n"
        h += f"    GDCLASS({prefixed_name}, RefCounted);\n\n"
        struct_map = {name: prefixed_name}
        for ftype, fname in fields:
            gt = godot_type(ftype, struct_map=struct_map, prefix=prefix)
            h += f"    {gt} {fname};\n"
        h += "\nprotected:\n"
        h += "    static void _bind_methods();\n\n"
        h += "public:\n"
        h += f"    {prefixed_name}();\n"
        for ftype, fname in fields:
            gt = godot_type(ftype, struct_map=struct_map, prefix=prefix)
            h += f"    void set_{fname}({gt} p_value) {{ {fname} = p_value; }}\n"
            h += f"    {gt} get_{fname}() const {{ return {fname}; }}\n"
        h += "};\n"

    h += f"\nclass {godot_class_name} : public RefCounted {{\n"
    h += f"    GDCLASS({godot_class_name}, RefCounted);\n\n"

    for cb_name, _, _ in data['callbacks']:
        var_name = "callback_" + cb_name
        h += f"    Callable {var_name};\n"

    h += "\nprotected:\n"
    h += "    static void _bind_methods();\n\npublic:\n"

    # Constants — 加前缀（仅整数）
    for name, val in data['defines'].items():
        if name.isupper() and any(k in name for k in ['ERROR','FLAG','MODE','STAGE']):
            if re.match(r'^-?\d+$|0x[0-9a-fA-F]+$', str(val)):
                prefixed_const = prefix.upper() + name
                h += f"    static const int {prefixed_const};\n"

    # Enums — 加前缀（带下划线）
    for enum_name, items in data['enums'].items():
        prefixed_enum = prefix + enum_name
        h += f"    enum {prefixed_enum} {{\n"
        for item, _ in items:
            prefixed_item = f"{prefix.upper()}_{enum_name.upper()}_{item.upper()}"
            h += f"        {prefixed_item},\n"
        h += "    };\n"

    # Functions
    for fname, ret, params in data['functions']:
        prefixed_fname = prefix + fname
        output_params = [(pt, pn) for pt, pn in params if is_output_param(pn, pt)]
        input_params = [(pt, pn) for pt, pn in params if not is_output_param(pn, pt)]

        if output_params or (ret in data['structs']):
            args = ", ".join([f"{godot_type(pt, prefix=prefix)} p_{pn}" for pt, pn in input_params])
            h += f"    Dictionary {prefixed_fname}({args});\n"
        else:
            args = ", ".join([f"{godot_type(pt, prefix=prefix)} p_{pn}" for pt, pn in params])
            ret_gd = godot_type(ret, prefix=prefix)
            h += f"    {ret_gd} {prefixed_fname}({args});\n"

    # Callback setters
    for cb_name, _, _ in data['callbacks']:
        setter = f"set_{cb_name.lower()}_callback"
        h += f"    void {setter}(Callable p_callback);\n"

    h += "};\n\n#endif\n"
    return h


def generate_cpp(module_name: str, data, prefix: str) -> str:
    godot_class_name = prefix + module_name.capitalize()
    header_file = re.sub(r'(?<!^)(?=[A-Z])', '_', module_name).lower()
    cpp = f"""#include "{header_file}.h"
#include "{os.path.basename(sys.argv[1])}"

"""

    # Structs
    for name, fields in data['structs'].items():
        prefixed_name = prefix + name
        cpp += f"\nvoid {prefixed_name}::_bind_methods() {{\n"
        for _, fname in fields:
            cpp += f'    ClassDB::bind_method(D_METHOD("set_{fname}", "value"), &{prefixed_name}::set_{fname});\n'
            cpp += f'    ClassDB::bind_method(D_METHOD("get_{fname}"), &{prefixed_name}::get_{fname});\n'
        cpp += "}\n"

        cpp += f"{prefixed_name}::{prefixed_name}() {{\n"
        struct_map = {name: prefixed_name}
        for ftype, fname in fields:
            gt = godot_type(ftype, struct_map=struct_map, prefix=prefix)
            if gt == 'int':
                cpp += f"    {fname} = 0;\n"
            elif gt == 'bool':
                cpp += f"    {fname} = false;\n"
            elif gt == 'double':
                cpp += f"    {fname} = 0.0;\n"
            elif gt == 'String':
                cpp += f"    {fname} = String();\n"
            elif gt.startswith('Ref<'):
                inner = gt[4:-1]
                cpp += f"    {fname} = Ref<{inner}>();\n"
            else:
                cpp += f"    // TODO: init {fname} of type {gt}\n"
        cpp += "}\n\n"

    # Constants
    constants = []
    for name, val in data['defines'].items():
        if name.isupper() and any(k in name for k in ['ERROR','FLAG','MODE','STAGE']):
            if re.match(r'^-?\d+$|0x[0-9a-fA-F]+$', str(val)):
                constants.append(name)

    if constants:
        cpp += "\n"
        for name in constants:
            prefixed_const = prefix.upper() + name
            cpp += f"const int {godot_class_name}::{prefixed_const} = {data['defines'][name]};\n"

    # Bind methods
    cpp += f"\nvoid {godot_class_name}::_bind_methods() {{\n"
    for name in constants:
        prefixed_const = prefix.upper() + name
        cpp += f'    BIND_CONSTANT({prefixed_const});\n'
    for enum_name, items in data['enums'].items():
        for item, _ in items:
            prefixed_item = f"{prefix.upper()}_{enum_name.upper()}_{item.upper()}"
            cpp += f'    BIND_ENUM_CONSTANT({prefixed_item});\n'

    for fname, ret, params in data['functions']:
        prefixed_fname = prefix + fname
        output_params = [(pt, pn) for pt, pn in params if is_output_param(pn, pt)]
        input_params = [(pt, pn) for pt, pn in params if not is_output_param(pn, pt)]
        arg_names = [pn for _, pn in (input_params if output_params or (ret in data['structs']) else params)]
        if arg_names:
            cpp += f'    ClassDB::bind_method(D_METHOD("{prefixed_fname}", "{", ".join(arg_names)}"), &{godot_class_name}::{prefixed_fname});\n'
        else:
            cpp += f'    ClassDB::bind_method(D_METHOD("{prefixed_fname}"), &{godot_class_name}::{prefixed_fname});\n'

    # Signals
    for cb_name, _, cb_params in data['callbacks']:
        signal_base = re.sub(r'([a-z])([A-Z])', r'\1_\2', cb_name.replace('Callback', '').replace('Listener', '')).lower()
        signal_name = prefix.lower() + signal_base
        param_types = [godot_type(pt, prefix=prefix) for pt, _ in cb_params]
        if param_types:
            props = [variant_type_to_property_info(t) for t in param_types]
            cpp += f'    ADD_SIGNAL(MethodInfo("{signal_name}", {", ".join(props)}));\n'
        else:
            cpp += f'    ADD_SIGNAL(MethodInfo("{signal_name}"));\n'

    cpp += "}\n\n"

    # Function implementations
    for fname, ret, params in data['functions']:
        prefixed_fname = prefix + fname
        output_params = [(pt, pn) for pt, pn in params if is_output_param(pn, pt)]
        input_params = [(pt, pn) for pt, pn in params if not is_output_param(pn, pt)]

        if output_params or (ret in data['structs']):
            args_sig = ", ".join([f"{godot_type(pt, prefix=prefix)} p_{pn}" for pt, pn in input_params])
            cpp += f"Dictionary {godot_class_name}::{prefixed_fname}({args_sig}) {{\n"
            cpp += "    Dictionary result;\n"

            # Declare C variables
            for pt, pn in params:
                if is_output_param(pn, pt):
                    if '[' in pt:
                        size_match = re.search(r'$$(\d+)$$', pt)
                        size = size_match.group(1) if size_match else "1024"
                        if 'char' in pt:
                            cpp += f"    char c_{pn}[{size}] = {{0}};\n"
                        else:
                            cpp += f"    uint8_t c_{pn}[{size}] = {{0}};\n"
                    elif '*' in pt:
                        if 'char' in pt:
                            cpp += f"    char c_{pn}[256] = {{0}};\n"
                        else:
                            cpp += f"    uint8_t c_{pn}[1024] = {{0}};\n"
                    else:
                        base_t = pt.replace('*', '').strip()
                        cpp += f"    {base_t} c_{pn} = {{0}};\n"
                else:
                    gt = godot_type(pt, prefix=prefix)
                    if gt == 'String':
                        cpp += f"    std::string s_{pn} = p_{pn}.utf8().get_data();\n"
                        cpp += f"    const char* c_{pn} = s_{pn}.c_str();\n"
                    elif gt == 'PackedByteArray':
                        cpp += f"    // TODO: handle PackedByteArray input for {pn}\n"
                        cpp += f"    uint8_t* c_{pn} = nullptr;\n"
                    else:
                        base_t = pt.replace('*', '').strip()
                        cpp += f"    auto c_{pn} = static_cast<{base_t}>(p_{pn});\n"

            call_args = [f"c_{pn}" for pt, pn in params]
            cpp += f"    auto ret_val = {fname}({', '.join(call_args)});\n"

            if ret in data['structs']:
                gd_ret_type = prefix + ret
                cpp += f"    Ref<{gd_ret_type}> ret_obj;\n"
                cpp += "    ret_obj.instantiate();\n"
                for ftype, fname_field in data['structs'][ret]:
                    cpp += f"    ret_obj->set_{fname_field}(ret_val.{fname_field});\n"
                cpp += "    result[\"value\"] = ret_obj;\n"
            else:
                cpp += "    result[\"error\"] = ret_val;\n"

            for pt, pn in output_params:
                if 'char' in pt:
                    cpp += f"    result[\"{pn}\"] = String(c_{pn});\n"
                elif '[' in pt or '*' in pt:
                    cpp += f"    // TODO: convert array output for {pn}\n"
                    cpp += f"    result[\"{pn}\"] = Variant();\n"
                else:
                    cpp += f"    result[\"{pn}\"] = c_{pn};\n"

            cpp += "    return result;\n}\n\n"

        else:
            args_sig = ", ".join([f"{godot_type(pt, prefix=prefix)} p_{pn}" for pt, pn in params])
            ret_gd = godot_type(ret, prefix=prefix)
            cpp += f"{ret_gd} {godot_class_name}::{prefixed_fname}({args_sig}) {{\n"
            call_args = []
            for pt, pn in params:
                gt = godot_type(pt, prefix=prefix)
                if gt == 'String':
                    cpp += f"    std::string s_{pn} = p_{pn}.utf8().get_data();\n"
                    call_args.append(f"s_{pn}.c_str()")
                else:
                    base_t = pt.replace('*', '').strip()
                    call_args.append(f"static_cast<{base_t}>(p_{pn})")
            cpp += f"    return {fname}({', '.join(call_args)});\n"
            cpp += "}\n\n"

    # Callback setters
    for cb_name, _, _ in data['callbacks']:
        var_name = "callback_" + cb_name
        setter = f"set_{cb_name.lower()}_callback"
        cpp += f"void {godot_class_name}::{setter}(Callable p_callback) {{\n"
        cpp += f"    {var_name} = p_callback;\n"
        cpp += "}\n\n"

    return cpp


# === 主程序 ===
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Generate GDExtension bindings from C header")
    parser.add_argument("header", help="Input C header file")
    parser.add_argument("module", help="Module name (e.g., 'limelight')")
    parser.add_argument("-p", "--prefix", default="", help="Prefix for all Godot-exposed identifiers (e.g., 'Li')")
    args = parser.parse_args()

    header_path = args.header
    module_name = args.module
    prefix = args.prefix

    # global parsed_data
    parsed_data = parse_header(header_path)

    h_code = generate_header(module_name, parsed_data, prefix)
    cpp_code = generate_cpp(module_name, parsed_data, prefix)

    safe_module_name = re.sub(r'(?<!^)(?=[A-Z])', '_', module_name).lower()

    with open(f"{safe_module_name}.h", 'w') as f:
        f.write(h_code)
    with open(f"{safe_module_name}.cpp", 'w') as f:
        f.write(cpp_code)

    print("✅ Generated full GDExtension module with advanced support:")
    if prefix:
        print(f"   Prefix: '{prefix}' applied to class, methods, signals, enums, constants, and structs")
    print("   - Struct returns as Ref<T>")
    print("   - Output parameters → Dictionary")
    print("   - Array/buffer handling (with TODOs for manual adjustment)")
    print("   - Callbacks as signals with correct PropertyInfo")
    print("\nFiles:")
    print(f"   {safe_module_name}.h")
    print(f"   {safe_module_name}.cpp")