#ifndef SODIUM_H
#define SODIUM_H

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// LIBSODIUM CRYPTOGRAPHIC CONSTANTS
// ============================================================================

// Secret key authenticated encryption (crypto_secretbox)
#define crypto_secretbox_KEYBYTES 32U
#define crypto_secretbox_NONCEBYTES 24U
#define crypto_secretbox_MACBYTES 16U

// ============================================================================
// LIBSODIUM FUNCTION DECLARATIONS
// ============================================================================

/**
 * @brief Initialize the libsodium library
 *
 * This function must be called before using any other libsodium functions.
 * It initializes the library's internal state and random number generator.
 *
 * @return 0 on success, -1 on failure
 */
int sodium_init(void);

/**
 * @brief Fill a buffer with cryptographically secure random bytes
 *
 * @param buf Pointer to the buffer to fill with random bytes
 * @param size Number of random bytes to generate
 */
void randombytes_buf(void *buf, size_t size);

/**
 * @brief Encrypt a message using secret-key authenticated encryption
 *
 * This is the "easy" interface that automatically handles padding and formatting.
 * The ciphertext will include the authentication tag (MAC).
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
                          const unsigned char *k);

/**
 * @brief Decrypt a message using secret-key authenticated encryption
 *
 * This is the "easy" interface that automatically handles padding and formatting.
 * The function will verify the authentication tag before decryption.
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
                               const unsigned char *k);

#ifdef __cplusplus
}
#endif

#endif // SODIUM_H