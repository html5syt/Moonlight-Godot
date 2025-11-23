#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/aes_context.hpp>
#include <godot_cpp/classes/crypto.hpp>
#include <godot_cpp/classes/crypto_key.hpp>
#include <godot_cpp/classes/hashing_context.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <vector>

using namespace godot;

class MoonlightExTools : public RefCounted {
    GDCLASS(MoonlightExTools, RefCounted);

protected:
    static void _bind_methods();

public:
    MoonlightExTools();
    ~MoonlightExTools();

    // 核心功能：生成配对响应
    String generate_pairing_response(String pin, String salt_hex, String server_challenge_hex, String client_pem_key);

private:
    // 辅助函数
    PackedByteArray hex_to_pba(const String &hex);
    String pba_to_hex(const PackedByteArray &pba);
    
    // --- 内部轻量级加密实现 ---
    struct MiniSHA256 {
        uint8_t data[64];
        uint32_t datalen;
        uint64_t bitlen;
        uint32_t state[8];
    };
    
    // 这里的关键修复：声明 transform 为成员函数
    void sha256_transform(MiniSHA256 *ctx, const uint8_t data[]);
    void sha256_init(MiniSHA256 *ctx);
    void sha256_update(MiniSHA256 *ctx, const uint8_t *data, size_t len);
    void sha256_final(MiniSHA256 *ctx, uint8_t hash[32]);
    
    void hmac_sha256(const uint8_t *key, size_t key_len, const uint8_t *data, size_t data_len, uint8_t out[32]);
    void pbkdf2_hmac_sha256(const uint8_t *pass, size_t pass_len, const uint8_t *salt, size_t salt_len, int iterations, uint32_t key_len, uint8_t *out_key);
};

