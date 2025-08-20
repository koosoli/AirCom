#include "include/crypto.h"

// ============================================================================
// NOTE: This is a PLACEHOLDER for an end-to-end encryption layer.
//
// This file is intended to be a wrapper for a real cryptographic library.
// The functions below demonstrate how to use libsodium for E2E encryption.
//
// NOTE: This code will NOT compile until the real libsodium library is added
// to the 'components/libsodium' directory.
// ============================================================================

#include "sodium.h" // This header will not be found until libsodium is added

// This would be a shared secret key derived from a key exchange, or a pre-shared key.
// WARNING: Do NOT use a hardcoded key in a production application.
static const unsigned char key[crypto_secretbox_KEYBYTES] = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
    0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
    0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20
};


std::vector<uint8_t> encrypt_message(const std::string& plaintext) {
    if (sodium_init() < 0) {
        // ESP_LOGE or similar would be here
        return {};
    }

    std::vector<uint8_t> ciphertext(crypto_secretbox_MACBYTES + plaintext.length());
    unsigned char nonce[crypto_secretbox_NONCEBYTES];

    randombytes_buf(nonce, sizeof(nonce));

    if (crypto_secretbox_easy(ciphertext.data(), (const unsigned char*)plaintext.c_str(), plaintext.length(), nonce, key) != 0) {
        // Encryption failed
        return {};
    }

    // Prepend the nonce to the ciphertext [NONCE][CIPHERTEXT]
    std::vector<uint8_t> final_payload;
    final_payload.insert(final_payload.end(), nonce, nonce + sizeof(nonce));
    final_payload.insert(final_payload.end(), ciphertext.begin(), ciphertext.end());

    return final_payload;
}


std::string decrypt_message(const std::vector<uint8_t>& payload) {
    if (sodium_init() < 0) {
        return "";
    }

    if (payload.size() < crypto_secretbox_NONCEBYTES + crypto_secretbox_MACBYTES) {
        return ""; // Message is too short to be valid
    }

    const unsigned char* nonce = payload.data();
    const unsigned char* ciphertext = payload.data() + crypto_secretbox_NONCEBYTES;
    size_t ciphertext_len = payload.size() - crypto_secretbox_NONCEBYTES;

    std::vector<uint8_t> decrypted(ciphertext_len - crypto_secretbox_MACBYTES);

    if (crypto_secretbox_open_easy(decrypted.data(), ciphertext, ciphertext_len, nonce, key) != 0) {
        // Decryption failed (invalid MAC - message may have been tampered with)
        return "";
    }

    return std::string((char*)decrypted.data(), decrypted.size());
}
