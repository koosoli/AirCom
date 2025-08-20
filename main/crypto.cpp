#include "include/crypto.h"

// ============================================================================
// NOTE: This is a PLACEHOLDER for an end-to-end encryption layer.
//
// The functions below provide a simple XOR cipher. This is **NOT SECURE** and
// is only intended to be a placeholder that simulates a real encryption layer.
// It allows the application's data flow to be developed and tested before
// a real cryptographic library (like libsodium or AES-GCM) is integrated.
// ============================================================================

// A simple, insecure key for the XOR cipher.
const uint8_t XOR_KEY[] = {0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F};

static void xor_cipher(const std::vector<uint8_t>& input, std::vector<uint8_t>& output) {
    output.clear();
    output.reserve(input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        output.push_back(input[i] ^ XOR_KEY[i % sizeof(XOR_KEY)]);
    }
}

std::vector<uint8_t> encrypt_message(const std::string& plaintext) {
    std::vector<uint8_t> input(plaintext.begin(), plaintext.end());
    std::vector<uint8_t> encrypted_output;
    xor_cipher(input, encrypted_output);
    return encrypted_output;
}


std::string decrypt_message(const std::vector<uint8_t>& ciphertext) {
    std::vector<uint8_t> decrypted_output;
    xor_cipher(ciphertext, decrypted_output);
    return std::string(decrypted_output.begin(), decrypted_output.end());
}
