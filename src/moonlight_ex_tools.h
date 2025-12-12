#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/aes_context.hpp>
#include <godot_cpp/classes/crypto.hpp>
#include <godot_cpp/classes/crypto_key.hpp>
#include <godot_cpp/classes/hashing_context.hpp>
#include <godot_cpp/classes/x509_certificate.hpp>
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

    // --- 原子加密 API (供 GDScript 调用) ---
    
    // 1. 从 PIN 和 Salt 生成 AES Key (PBKDF2)
    PackedByteArray derive_aes_key(String pin, String salt_hex);
    
    // 2. AES-128-ECB 加密 (用于 ClientChallenge)
    String encrypt_aes_hex(PackedByteArray data, PackedByteArray key);
    
    // 3. AES-128-ECB 解密 (用于 ServerResponse)
    PackedByteArray decrypt_aes_hex(String hex_data, PackedByteArray key);
    
    // 4. RSA-SHA256 签名 (用于 ClientPairingSecret)
    PackedByteArray sign_data(PackedByteArray data, String key_pem);
    
    // 5. 验证签名 (用于防止 MITM)
    bool verify_signature(PackedByteArray data, PackedByteArray signature, String cert_pem);

    // 6. 生成随机字节 (辅助)
    PackedByteArray generate_random_bytes(int size);

private:
    // 辅助：Hex/Bytes 转换
    PackedByteArray hex_to_pba(const String &hex);
    String pba_to_hex(const PackedByteArray &pba);
    
    // 内部轻量级加密实现 (无需 OpenSSL)
    struct MiniSHA256 {
        uint8_t data[64];
        uint32_t datalen;
        uint64_t bitlen;
        uint32_t state[8];
    };
    
    void sha256_init(MiniSHA256 *ctx);
    void sha256_transform(MiniSHA256 *ctx, const uint8_t data[]);
    void sha256_update(MiniSHA256 *ctx, const uint8_t *data, size_t len);
    void sha256_final(MiniSHA256 *ctx, uint8_t hash[32]);
    void hmac_sha256(const uint8_t *key, size_t key_len, const uint8_t *data, size_t data_len, uint8_t out[32]);
    void pbkdf2_hmac_sha256(const uint8_t *pass, size_t pass_len, const uint8_t *salt, size_t salt_len, int iterations, uint32_t key_len, uint8_t *out_key);
};