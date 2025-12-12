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
    ClassDB::bind_method(D_METHOD("derive_aes_key", "pin", "salt_hex"), &MoonlightExTools::derive_aes_key);
    ClassDB::bind_method(D_METHOD("encrypt_aes_hex", "data", "key"), &MoonlightExTools::encrypt_aes_hex);
    ClassDB::bind_method(D_METHOD("decrypt_aes_hex", "hex_data", "key"), &MoonlightExTools::decrypt_aes_hex);
    ClassDB::bind_method(D_METHOD("sign_data", "data", "key_pem"), &MoonlightExTools::sign_data);
    ClassDB::bind_method(D_METHOD("verify_signature", "data", "signature", "cert_pem"), &MoonlightExTools::verify_signature);
    ClassDB::bind_method(D_METHOD("generate_random_bytes", "size"), &MoonlightExTools::generate_random_bytes);
}

// 1. PBKDF2 生成密钥
PackedByteArray MoonlightExTools::derive_aes_key(String pin, String salt_hex) {
    PackedByteArray salt = hex_to_pba(salt_hex);
    CharString pin_utf8 = pin.utf8();
    
    uint8_t key_raw[16];
    // 10000 次迭代，生成 16 字节 (AES-128)
    pbkdf2_hmac_sha256((const uint8_t*)pin_utf8.get_data(), pin_utf8.length(), 
                       salt.ptr(), salt.size(), 10000, 16, key_raw);
    
    PackedByteArray key;
    key.resize(16);
    memcpy(key.ptrw(), key_raw, 16);
    return key;
}

// 2. AES 加密
String MoonlightExTools::encrypt_aes_hex(PackedByteArray data, PackedByteArray key) {
    Ref<AESContext> aes = memnew(AESContext);
    aes->start(AESContext::MODE_ECB_ENCRYPT, key);
    // 确保数据是 16 的倍数 (Moonlight 协议通常处理定长数据，不足需补齐，但握手阶段数据通常已对齐)
    PackedByteArray encrypted = aes->update(data);
    aes->finish();
    return pba_to_hex(encrypted);
}

// 3. AES 解密
PackedByteArray MoonlightExTools::decrypt_aes_hex(String hex_data, PackedByteArray key) {
    PackedByteArray data = hex_to_pba(hex_data);
    Ref<AESContext> aes = memnew(AESContext);
    aes->start(AESContext::MODE_ECB_DECRYPT, key);
    PackedByteArray plain = aes->update(data);
    aes->finish();
    return plain;
}

// 4. RSA 签名
PackedByteArray MoonlightExTools::sign_data(PackedByteArray data, String key_pem) {
    Ref<Crypto> crypto = memnew(Crypto);
    Ref<CryptoKey> key = memnew(CryptoKey);
    
    if (key->load_from_string(key_pem) != OK) {
        UtilityFunctions::printerr("Failed to load private key");
        return PackedByteArray();
    }
    
    // 先做 SHA256 Hash
    Ref<HashingContext> ctx = memnew(HashingContext);
    ctx->start(HashingContext::HASH_SHA256);
    ctx->update(data);
    PackedByteArray digest = ctx->finish();
    
    return crypto->sign(HashingContext::HASH_SHA256, digest, key);
}

// 5. 验证签名
bool MoonlightExTools::verify_signature(PackedByteArray data, PackedByteArray signature, String cert_pem) {
    Ref<Crypto> crypto = memnew(Crypto);
    Ref<X509Certificate> cert = memnew(X509Certificate);
    
    if (cert->load_from_string(cert_pem) != OK) {
        UtilityFunctions::printerr("Failed to load certificate");
        return false;
    }
    
    Ref<CryptoKey> pub_key = cert->get_public_key(); // Godot 4.5+ method? 
    // 注意：Godot API 对 verify 的支持有限。如果直接 verify 不可行，
    // 通常 Pairing 阶段最重要的是用自己的私钥签名发给服务器。
    // 验证服务器签名是为了防止 MITM。
    
    // 计算 Hash
    Ref<HashingContext> ctx = memnew(HashingContext);
    ctx->start(HashingContext::HASH_SHA256);
    ctx->update(data);
    PackedByteArray digest = ctx->finish();
    
    return crypto->verify(HashingContext::HASH_SHA256, digest, signature, pub_key);
}

PackedByteArray MoonlightExTools::generate_random_bytes(int size) {
    Ref<Crypto> c = memnew(Crypto);
    return c->generate_random_bytes(size);
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
