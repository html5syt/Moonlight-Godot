#include "moonlight_ex_tools.h"
#include <godot_cpp/variant/utility_functions.hpp>
#include <cstring>
#include <cstdio> // 用于 sscanf, snprintf

using namespace godot;

// --- SHA256 常量 ---
static const uint32_t k[64] = {
	0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
	0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
	0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
	0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
	0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
	0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
	0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
	0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

// --- SHA256 宏定义 ---
#define ROR(x, y) (((x) >> (y)) | ((x) << (32 - (y))))
#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROR(x, 2) ^ ROR(x, 13) ^ ROR(x, 22))
#define EP1(x) (ROR(x, 6) ^ ROR(x, 11) ^ ROR(x, 25))
#define SIG0(x) (ROR(x, 7) ^ ROR(x, 18) ^ ((x) >> 3))
#define SIG1(x) (ROR(x, 17) ^ ROR(x, 19) ^ ((x) >> 10))

MoonlightExTools::MoonlightExTools() {}
MoonlightExTools::~MoonlightExTools() {}

void MoonlightExTools::_bind_methods() {
    ClassDB::bind_method(D_METHOD("generate_pairing_response", "pin", "salt_hex", "server_challenge_hex", "client_pem_key"), &MoonlightExTools::generate_pairing_response);
}

// --- 核心对外接口 ---

String MoonlightExTools::generate_pairing_response(String pin, String salt_hex, String server_challenge_hex, String client_pem_key) {
    // 1. 数据转换
    PackedByteArray salt = hex_to_pba(salt_hex);
    PackedByteArray server_challenge_enc = hex_to_pba(server_challenge_hex);
    CharString pin_utf8 = pin.utf8();

    // 2. PBKDF2 派生 AES 密钥
    uint8_t aes_key_raw[16];
    pbkdf2_hmac_sha256((const uint8_t*)pin_utf8.get_data(), pin_utf8.length(), 
                       salt.ptr(), salt.size(), 
                       10000, 16, aes_key_raw);
    
    PackedByteArray aes_key;
    aes_key.resize(16);
    memcpy(aes_key.ptrw(), aes_key_raw, 16);

    // 3. AES 解密 Server Challenge
    Ref<AESContext> aes = memnew(AESContext);
    aes->start(AESContext::MODE_ECB_DECRYPT, aes_key);
    PackedByteArray server_challenge_plain = aes->update(server_challenge_enc);
    aes->finish();

    if (server_challenge_plain.size() != 16) {
        UtilityFunctions::printerr("MoonlightExTools: Decrypted challenge size mismatch.");
        return "";
    }

    // 4. 生成 Client Secret
    Ref<Crypto> crypto = memnew(Crypto);
    PackedByteArray client_secret = crypto->generate_random_bytes(16);

    // 5. 准备签名
    PackedByteArray payload_to_sign;
    payload_to_sign.append_array(server_challenge_plain);
    payload_to_sign.append_array(client_secret);

    // 6. RSA 签名
    Ref<HashingContext> hash_ctx = memnew(HashingContext);
    hash_ctx->start(HashingContext::HASH_SHA256);
    hash_ctx->update(payload_to_sign);
    PackedByteArray digest = hash_ctx->finish();

    Ref<CryptoKey> key = memnew(CryptoKey);
    if (key->load_from_string(client_pem_key) != OK) {
        UtilityFunctions::printerr("MoonlightExTools: Failed to load private key.");
        return "";
    }

    PackedByteArray signature = crypto->sign(HashingContext::HASH_SHA256, digest, key);
    if (signature.is_empty()) {
        UtilityFunctions::printerr("MoonlightExTools: Signature generation failed.");
        return "";
    }

    // 7. 构造响应
    PackedByteArray response_payload;
    response_payload.append_array(client_secret);
    response_payload.append_array(signature);
    
    // 8. AES 加密响应
    aes->start(AESContext::MODE_ECB_ENCRYPT, aes_key);
    PackedByteArray response_enc = aes->update(response_payload);
    aes->finish();

    return pba_to_hex(response_enc);
}

// --- 辅助函数 ---

PackedByteArray MoonlightExTools::hex_to_pba(const String &hex) {
    PackedByteArray bytes;
    CharString cs = hex.ascii();
    const char *input = cs.get_data();
    size_t len = strlen(input);
    bytes.resize(len / 2);
    uint8_t *w = bytes.ptrw();
    
    for (size_t i = 0; i < len; i += 2) {
        unsigned int x;
        sscanf(input + i, "%02x", &x);
        w[i/2] = (uint8_t)x;
    }
    return bytes;
}

String MoonlightExTools::pba_to_hex(const PackedByteArray &pba) {
    String ret = "";
    const uint8_t *r = pba.ptr();
    char buf[3];
    for (int i = 0; i < pba.size(); i++) {
        snprintf(buf, sizeof(buf), "%02x", r[i]);
        ret += buf;
    }
    return ret;
}

// --- 内部 SHA256 / HMAC / PBKDF2 实现 ---

void MoonlightExTools::sha256_init(MiniSHA256 *ctx) {
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0] = 0x6a09e667; ctx->state[1] = 0xbb67ae85;
    ctx->state[2] = 0x3c6ef372; ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f; ctx->state[5] = 0x9b05688c;
    ctx->state[6] = 0x1f83d9ab; ctx->state[7] = 0x5be0cd19;
}

// 修复：添加了类作用域限定符 MoonlightExTools::
void MoonlightExTools::sha256_transform(MiniSHA256 *ctx, const uint8_t data[]) {
    uint32_t a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];
    for (i = 0, j = 0; i < 16; ++i, j += 4)
        m[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) | (data[j + 3]);
    for (; i < 64; ++i)
        m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];

    a = ctx->state[0]; b = ctx->state[1]; c = ctx->state[2]; d = ctx->state[3];
    e = ctx->state[4]; f = ctx->state[5]; g = ctx->state[6]; h = ctx->state[7];

    for (i = 0; i < 64; ++i) {
        t1 = h + EP1(e) + CH(e, f, g) + k[i] + m[i];
        t2 = EP0(a) + MAJ(a, b, c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }
    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

void MoonlightExTools::sha256_update(MiniSHA256 *ctx, const uint8_t *data, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        ctx->data[ctx->datalen] = data[i];
        ctx->datalen++;
        if (ctx->datalen == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

void MoonlightExTools::sha256_final(MiniSHA256 *ctx, uint8_t hash[32]) {
    uint32_t i = ctx->datalen;
    if (ctx->datalen < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56) ctx->data[i++] = 0x00;
    } else {
        ctx->data[i++] = 0x80;
        while (i < 64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }
    ctx->bitlen += ctx->datalen * 8;
    ctx->data[63] = ctx->bitlen;
    ctx->data[62] = ctx->bitlen >> 8;
    ctx->data[61] = ctx->bitlen >> 16;
    ctx->data[60] = ctx->bitlen >> 24;
    ctx->data[59] = ctx->bitlen >> 32;
    ctx->data[58] = ctx->bitlen >> 40;
    ctx->data[57] = ctx->bitlen >> 48;
    ctx->data[56] = ctx->bitlen >> 56;
    sha256_transform(ctx, ctx->data);
    for (i = 0; i < 4; ++i) {
        hash[i]      = (ctx->state[0] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 4]  = (ctx->state[1] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 8]  = (ctx->state[2] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 20] = (ctx->state[5] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 24] = (ctx->state[6] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 28] = (ctx->state[7] >> (24 - i * 8)) & 0x000000ff;
    }
}

void MoonlightExTools::hmac_sha256(const uint8_t *key, size_t key_len, const uint8_t *data, size_t data_len, uint8_t out[32]) {
    uint8_t k_ipad[64];
    uint8_t k_opad[64];
    uint8_t tk[32];
    
    // 如果 key 过长，先 hash
    if (key_len > 64) {
        MiniSHA256 ctx;
        sha256_init(&ctx);
        sha256_update(&ctx, key, key_len);
        sha256_final(&ctx, tk);
        key = tk;
        key_len = 32;
    }

    memset(k_ipad, 0x36, 64);
    memset(k_opad, 0x5c, 64);

    for (size_t i = 0; i < key_len; i++) {
        k_ipad[i] ^= key[i];
        k_opad[i] ^= key[i];
    }

    MiniSHA256 ctx; // 使用单一的 ctx 变量
    
    // Inner
    sha256_init(&ctx);
    sha256_update(&ctx, k_ipad, 64);
    sha256_update(&ctx, data, data_len);
    sha256_final(&ctx, out);

    // Outer
    sha256_init(&ctx);
    sha256_update(&ctx, k_opad, 64);
    sha256_update(&ctx, out, 32);
    sha256_final(&ctx, out);
}

void MoonlightExTools::pbkdf2_hmac_sha256(const uint8_t *pass, size_t pass_len, const uint8_t *salt, size_t salt_len, int iterations, uint32_t key_len, uint8_t *out_key) {
    uint32_t i = 1;
    uint8_t digest[32];
    uint8_t block[32];
    uint8_t salt_block[128]; // 确保足够大
    size_t salt_block_len;

    uint32_t klen = 0;
    while (klen < key_len) {
        // U1 = PRF(P, S || INT(i))
        if (salt_len + 4 > sizeof(salt_block)) return; // 安全检查

        memcpy(salt_block, salt, salt_len);
        salt_block[salt_len] = (i >> 24) & 0xFF;
        salt_block[salt_len+1] = (i >> 16) & 0xFF;
        salt_block[salt_len+2] = (i >> 8) & 0xFF;
        salt_block[salt_len+3] = i & 0xFF;
        salt_block_len = salt_len + 4;

        hmac_sha256(pass, pass_len, salt_block, salt_block_len, digest);
        memcpy(block, digest, 32);

        // U2 .. Uc
        for (int iter = 1; iter < iterations; iter++) {
            hmac_sha256(pass, pass_len, digest, 32, digest);
            for (int j = 0; j < 32; j++) block[j] ^= digest[j];
        }

        size_t copy_len = (key_len - klen > 32) ? 32 : (key_len - klen);
        memcpy(out_key + klen, block, copy_len);
        klen += copy_len;
        i++;
    }
}
