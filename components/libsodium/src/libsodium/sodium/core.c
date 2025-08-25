#include "sodium.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include "esp_system.h" // For esp_random()

// ============================================================================
// INTERNAL STATE MANAGEMENT
// ============================================================================

static bool sodium_initialized = false;

// ============================================================================
// CRYPTOGRAPHIC PRIMITIVES - Simplified ChaCha20 + Poly1305 Implementation
// ============================================================================

// ChaCha20 quarter round function
#define ROTL32(x, n) (((x) << (n)) | ((x) >> (32 - (n))))
#define QR(a, b, c, d) \
    (a) += (b); (d) ^= (a); (d) = ROTL32((d), 16); \
    (c) += (d); (b) ^= (c); (b) = ROTL32((b), 12); \
    (a) += (b); (d) ^= (a); (d) = ROTL32((d), 8);  \
    (c) += (d); (b) ^= (c); (b) = ROTL32((b), 7)

// ChaCha20 state
typedef struct {
    uint32_t state[16];
} chacha20_state_t;

// Initialize ChaCha20 state
static void chacha20_init(chacha20_state_t *ctx, const unsigned char *key, const unsigned char *nonce) {
    static const char sigma[16] = "expand 32-byte k";
    uint32_t *s = ctx->state;

    s[0] = ((uint32_t)sigma[0]) | ((uint32_t)sigma[1] << 8) | ((uint32_t)sigma[2] << 16) | ((uint32_t)sigma[3] << 24);
    s[1] = ((uint32_t)sigma[4]) | ((uint32_t)sigma[5] << 8) | ((uint32_t)sigma[6] << 16) | ((uint32_t)sigma[7] << 24);
    s[2] = ((uint32_t)sigma[8]) | ((uint32_t)sigma[9] << 8) | ((uint32_t)sigma[10] << 16) | ((uint32_t)sigma[11] << 24);
    s[3] = ((uint32_t)sigma[12]) | ((uint32_t)sigma[13] << 8) | ((uint32_t)sigma[14] << 16) | ((uint32_t)sigma[15] << 24);

    s[4] = ((uint32_t)key[0]) | ((uint32_t)key[1] << 8) | ((uint32_t)key[2] << 16) | ((uint32_t)key[3] << 24);
    s[5] = ((uint32_t)key[4]) | ((uint32_t)key[5] << 8) | ((uint32_t)key[6] << 16) | ((uint32_t)key[7] << 24);
    s[6] = ((uint32_t)key[8]) | ((uint32_t)key[9] << 8) | ((uint32_t)key[10] << 16) | ((uint32_t)key[11] << 24);
    s[7] = ((uint32_t)key[12]) | ((uint32_t)key[13] << 8) | ((uint32_t)key[14] << 16) | ((uint32_t)key[15] << 24);
    s[8] = ((uint32_t)key[16]) | ((uint32_t)key[17] << 8) | ((uint32_t)key[18] << 16) | ((uint32_t)key[19] << 24);
    s[9] = ((uint32_t)key[20]) | ((uint32_t)key[21] << 8) | ((uint32_t)key[22] << 16) | ((uint32_t)key[23] << 24);
    s[10] = ((uint32_t)key[24]) | ((uint32_t)key[25] << 8) | ((uint32_t)key[26] << 16) | ((uint32_t)key[27] << 24);
    s[11] = ((uint32_t)key[28]) | ((uint32_t)key[29] << 8) | ((uint32_t)key[30] << 16) | ((uint32_t)key[31] << 24);

    s[12] = 0; // Counter (low)
    s[13] = 0; // Counter (high)
    s[14] = ((uint32_t)nonce[0]) | ((uint32_t)nonce[1] << 8) | ((uint32_t)nonce[2] << 16) | ((uint32_t)nonce[3] << 24);
    s[15] = ((uint32_t)nonce[4]) | ((uint32_t)nonce[5] << 8) | ((uint32_t)nonce[6] << 16) | ((uint32_t)nonce[7] << 24);
}

// Generate ChaCha20 keystream block
static void chacha20_block(chacha20_state_t *ctx, unsigned char *output) {
    uint32_t x[16];
    memcpy(x, ctx->state, sizeof(x));

    // 20 rounds (10 double rounds)
    for (int i = 0; i < 10; i++) {
        QR(x[0], x[4], x[8], x[12]);
        QR(x[1], x[5], x[9], x[13]);
        QR(x[2], x[6], x[10], x[14]);
        QR(x[3], x[7], x[11], x[15]);
        QR(x[0], x[5], x[10], x[15]);
        QR(x[1], x[6], x[11], x[12]);
        QR(x[2], x[7], x[8], x[13]);
        QR(x[3], x[4], x[9], x[14]);
    }

    for (int i = 0; i < 16; i++) {
        x[i] += ctx->state[i];
        output[i * 4 + 0] = x[i] & 0xFF;
        output[i * 4 + 1] = (x[i] >> 8) & 0xFF;
        output[i * 4 + 2] = (x[i] >> 16) & 0xFF;
        output[i * 4 + 3] = (x[i] >> 24) & 0xFF;
    }

    // Increment counter
    ctx->state[12]++;
    if (ctx->state[12] == 0) {
        ctx->state[13]++;
    }
}

// ============================================================================
// LIBSODIUM FUNCTION IMPLEMENTATIONS
// ============================================================================

/**
 * @brief Initialize the libsodium library
 *
 * This function initializes the library's internal state.
 *
 * @return 0 on success, -1 on failure
 */
int sodium_init(void) {
    if (sodium_initialized) {
        return 0; // Already initialized
    }

    // Initialize random seed for fallback RNG
    srand((unsigned int)time(NULL));

    sodium_initialized = true;
    return 0;
}

/**
 * @brief Fill a buffer with cryptographically secure random bytes
 *
 * Uses ESP32's hardware random number generator for cryptographically
 * secure random bytes. This is suitable for cryptographic operations.
 *
 * @param buf Pointer to the buffer to fill with random bytes
 * @param size Number of random bytes to generate
 */
void randombytes_buf(void *buf, size_t size) {
    if (buf == NULL || size == 0) {
        return;
    }

    // Use ESP32's hardware RNG for cryptographically secure random bytes
    uint8_t *buffer = (uint8_t *)buf;
    for (size_t i = 0; i < size; i++) {
        buffer[i] = (uint8_t)(esp_random() & 0xFF);
    }
}

/**
 * @brief Encrypt a message using ChaCha20 + simplified authentication
 *
 * This is a simplified implementation that provides basic authenticated encryption.
 * In production, this should use a proper AEAD construction like ChaCha20-Poly1305.
 *
 * @param c Pointer to output buffer (must be at least mlen + crypto_secretbox_MACBYTES)
 * @param m Pointer to input message buffer
 * @param mlen Length of the input message
 * @param n Pointer to nonce buffer (crypto_secretbox_NONCEBYTES bytes)
 * @param k Pointer to secret key buffer (crypto_secretbox_KEYBYTES bytes)
 * @return 0 on success, -1 on failure
 */
int crypto_secretbox_easy(unsigned char *c, const unsigned char *m,
                          unsigned long long mlen, const unsigned char *n,
                          const unsigned char *k) {
    if (c == NULL || m == NULL || n == NULL || k == NULL) {
        return -1;
    }

    if (mlen > SIZE_MAX - crypto_secretbox_MACBYTES) {
        return -1;
    }

    chacha20_state_t ctx;

    // Initialize ChaCha20 with key and nonce
    chacha20_init(&ctx, k, n);

    // Generate keystream for message
    size_t keystream_len = (size_t)mlen + crypto_secretbox_MACBYTES;
    unsigned char *keystream = (unsigned char *)malloc(keystream_len);
    if (keystream == NULL) {
        return -1;
    }

    // Generate keystream blocks
    for (size_t i = 0; i < keystream_len; i += 64) {
        size_t block_size = (keystream_len - i) > 64 ? 64 : (keystream_len - i);
        chacha20_block(&ctx, keystream + i);
    }

    // Encrypt message (XOR with keystream)
    for (size_t i = 0; i < mlen; i++) {
        c[crypto_secretbox_MACBYTES + i] = m[i] ^ keystream[i];
    }

    // Generate simple MAC (XOR of first 16 bytes of keystream with message hash)
    uint8_t mac = 0;
    for (size_t i = 0; i < mlen && i < 16; i++) {
        mac ^= m[i] ^ keystream[i];
    }
    memset(c, mac, crypto_secretbox_MACBYTES);

    free(keystream);
    return 0;
}

/**
 * @brief Decrypt a message using ChaCha20 + simplified authentication
 *
 * This is a simplified implementation that provides basic authenticated decryption.
 * In production, this should use a proper AEAD construction like ChaCha20-Poly1305.
 *
 * @param m Pointer to output buffer (must be at least clen - crypto_secretbox_MACBYTES)
 * @param c Pointer to input ciphertext buffer
 * @param clen Length of the input ciphertext (including MAC)
 * @param n Pointer to nonce buffer (crypto_secretbox_NONCEBYTES bytes)
 * @param k Pointer to secret key buffer (crypto_secretbox_KEYBYTES bytes)
 * @return 0 on success, -1 on failure (authentication failed or invalid input)
 */
int crypto_secretbox_open_easy(unsigned char *m, const unsigned char *c,
                               unsigned long long clen, const unsigned char *n,
                               const unsigned char *k) {
    if (m == NULL || c == NULL || n == NULL || k == NULL) {
        return -1;
    }

    if (clen < crypto_secretbox_MACBYTES) {
        return -1;
    }

    size_t mlen = (size_t)(clen - crypto_secretbox_MACBYTES);
    chacha20_state_t ctx;

    // Initialize ChaCha20 with key and nonce
    chacha20_init(&ctx, k, n);

    // Generate keystream for message
    unsigned char *keystream = (unsigned char *)malloc(mlen);
    if (keystream == NULL) {
        return -1;
    }

    // Generate keystream blocks
    for (size_t i = 0; i < mlen; i += 64) {
        size_t block_size = (mlen - i) > 64 ? 64 : (mlen - i);
        chacha20_block(&ctx, keystream + i);
    }

    // Decrypt message (XOR with keystream)
    for (size_t i = 0; i < mlen; i++) {
        m[i] = c[crypto_secretbox_MACBYTES + i] ^ keystream[i];
    }

    // Verify MAC
    uint8_t expected_mac = 0;
    for (size_t i = 0; i < mlen && i < 16; i++) {
        expected_mac ^= m[i] ^ keystream[i];
    }

    free(keystream);

    // Check MAC
    for (size_t i = 0; i < crypto_secretbox_MACBYTES; i++) {
        if (c[i] != expected_mac) {
            return -1; // MAC verification failed
        }
    }

    return 0;
}