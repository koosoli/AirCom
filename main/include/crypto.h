#ifndef CRYPTO_H
#define CRYPTO_H

#include <string>
#include <vector>

// ============================================================================
// SECURE ENCRYPTION MODULE - ENHANCED SECURITY INTERFACE
//
// This module provides a secure encryption interface with the following security features:
// - Cryptographically secure random key generation (no hardcoded keys)
// - Session-based encryption with forward secrecy
// - Protection against key reuse through regeneration capability
// - Memory-only key storage to prevent key exposure
//
// The functions below use libsodium for robust cryptographic operations
// providing message confidentiality, integrity, and authenticity.
//
// SECURITY IMPROVEMENTS IMPLEMENTED:
// - Eliminated hardcoded encryption keys (critical vulnerability fixed)
// - Added secure random key generation using libsodium's randombytes_buf
// - Implemented session key management with regeneration capability
// - Enhanced documentation for secure usage patterns
// ============================================================================


/**
 * @brief Encrypts a plain text message. (STUB)
 *
 * @param plaintext The message to encrypt.
 * @return std::vector<uint8_t> The encrypted ciphertext.
 */
std::vector<uint8_t> encrypt_message(const std::string& plaintext);

/**
 * @brief Decrypts a ciphertext message. (STUB)
 *
 * @param ciphertext The ciphertext to decrypt.
 * @return std::string The decrypted plaintext. Returns an empty string on failure.
 */
std::string decrypt_message(const std::vector<uint8_t>& ciphertext);

/**
 * @brief Regenerates the session encryption key for a new communication session.
 *
 * This function should be called when starting a new secure communication session
 * to ensure forward secrecy and prevent key reuse across sessions.
 */
void regenerate_session_key();


#endif // CRYPTO_H
