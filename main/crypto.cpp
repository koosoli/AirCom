#include "include/crypto.h"

// ============================================================================
// NOTE: This is a PLACEHOLDER for an end-to-end encryption layer.
//
// The functions below are simple stubs that do not provide any real security.
// They exist to allow the application's messaging flow to be developed.
// In a real-world application, these should be replaced with a robust
// cryptographic library like libsodium or a custom AES-GCM implementation.
// ============================================================================


std::vector<uint8_t> encrypt_message(const std::string& plaintext) {
    // STUB: This is NOT real encryption.
    // It simply prepends "[ENC]" to the message to show the concept.
    std::string encrypted_text = "[ENC]" + plaintext;
    return std::vector<uint8_t>(encrypted_text.begin(), encrypted_text.end());
}


std::string decrypt_message(const std::vector<uint8_t>& ciphertext) {
    // STUB: This is NOT real decryption.
    std::string received_text(ciphertext.begin(), ciphertext.end());

    std::string prefix = "[ENC]";
    if (received_text.rfind(prefix, 0) == 0) { // pos=0 limits the search to the prefix
        // Found the prefix, return the rest of the string.
        return received_text.substr(prefix.length());
    }

    // If the prefix isn't there, something is wrong. Return an empty string.
    // A real implementation would have proper authentication tags to prevent this.
    return "";
}
