#include "example_class.h"

void ExampleClass::_bind_methods() {
	godot::ClassDB::bind_method(D_METHOD("print_type", "variant"), &ExampleClass::print_type);
	godot::ClassDB::bind_method(D_METHOD("print_helloworld"), &ExampleClass::print_helloworld);
	godot::ClassDB::bind_method(D_METHOD("print_fibonacci", "n"), &ExampleClass::print_fibonacci);
}

void ExampleClass::print_type(const Variant &p_variant) const {
	print_line(vformat("Type: %d", p_variant.get_type()));
}

void ExampleClass::print_helloworld() const {
	print_line("Hello, Wor444ld!你好，世界！");
}

long long fibonacci(int n) {
	if (n < 0) {
		print_line("输入应为非负整数");
		return 0;
	}

	if (n <= 1) {
		return n;
	}

	long long prev = 0;
	long long curr = 1;

	for (int i = 2; i <= n; ++i) {
		long long next = prev + curr;
		prev = curr;
		curr = next;
	}

	return curr;
}

void ExampleClass::print_fibonacci(int n) const {
	print_line(vformat("Fibonacci(%d) = %lld", n, fibonacci(n)));
	print_line(vformat("Fibonacci({}) = {}", n, fibonacci(n)));
	print_line(vformat("Fibonacci(%d) = %d", n, Variant(fibonacci(n))));
	print_line(fibonacci(n));
}