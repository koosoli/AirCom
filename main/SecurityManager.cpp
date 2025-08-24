#include "include/SecurityManager.h"
#include "include/config.h" // For TAG
#include "esp_log.h"
#include "mbedtls/error.h" // For error codes, assuming mbedtls from ESP-IDF
#include "mbedtls/gcm.h"   // Using AES-GCM for authenticated encryption

// TODO: Replace mbedtls with libsodium once it's integrated as a component.
// Libsodium is generally preferred for its simpler and safer API.

SecurityManager::SecurityManager() : isInitialized(false) {
    // Default constructor
}

SecurityManager::~SecurityManager() {
    // Destructor
}

bool SecurityManager::begin() {
    ESP_LOGI(TAG, "Initializing Security Manager...");

    // In a real application, the key would be loaded from secure storage or derived.
    // For this placeholder, we use a hardcoded key.
    // WARNING: This is insecure and for development purposes only.
    const char* default_key = "THIS_IS_A_DEFAULT_32_BYTE_KEY!!";
    groupKey.assign(default_key, default_key + 32);

    // TODO: Initialize the underlying cryptographic library (e.g., libsodium)
    // if (sodium_init() < 0) {
    //     ESP_LOGE(TAG, "Failed to initialize libsodium!");
    //     isInitialized = false;
    //     return false;
    // }

    isInitialized = true;
    ESP_LOGI(TAG, "Security Manager initialized.");
    ESP_LOGW(TAG, "Using insecure default group key!");
    return true;
}

void SecurityManager::setGroupKey(const std::vector<uint8_t>& new_key) {
    if (new_key.size() != 32) { // Assuming 256-bit key
        ESP_LOGE(TAG, "Invalid key size. Must be 32 bytes.");
        return;
    }
    groupKey = new_key;
    ESP_LOGI(TAG, "Group key has been updated.");
}

bool SecurityManager::encrypt(const std::vector<uint8_t>& plaintext, EncryptedPacket& encrypted_packet) {
    if (!isInitialized) {
        ESP_LOGE(TAG, "Security Manager not initialized.");
        return false;
    }

    // TODO: Implement encryption using libsodium's crypto_aead_xchacha20poly1305_ietf_encrypt
    // 1. Generate a random nonce.
    // 2. Call the encryption function.
    // 3. Populate the encrypted_packet with the nonce and ciphertext.

    ESP_LOGD(TAG, "Placeholder: Encrypting %d bytes.", plaintext.size());
    // Placeholder logic:
    // This is NOT real encryption. It just copies the plaintext and adds a fake nonce.
    std::string fake_nonce = "fake_nonce";
    // encrypted_packet.set_nonce(fake_nonce.c_str(), fake_nonce.length());
    // encrypted_packet.set_ciphertext(plaintext.data(), plaintext.size());

    ESP_LOGW(TAG, "Encryption is a placeholder and NOT secure!");
    return true;
}

bool SecurityManager::decrypt(const EncryptedPacket& encrypted_packet, std::vector<uint8_t>& plaintext) {
    if (!isInitialized) {
        ESP_LOGE(TAG, "Security Manager not initialized.");
        return false;
    }

    // TODO: Implement decryption using libsodium's crypto_aead_xchacha20poly1305_ietf_decrypt
    // 1. Extract nonce and ciphertext from the encrypted_packet.
    // 2. Call the decryption function.
    // 3. If successful, populate the plaintext vector.
    // 4. If it fails, the packet is forged or corrupt; discard it.

    ESP_LOGD(TAG, "Placeholder: Decrypting data.");
    // Placeholder logic:
    // This is NOT real decryption. It just copies the ciphertext to the plaintext.
    // const std::string& ciphertext = encrypted_packet.ciphertext();
    // plaintext.assign(ciphertext.begin(), ciphertext.end());

    ESP_LOGW(TAG, "Decryption is a placeholder and NOT secure!");
    return true;
}

bool SecurityManager::initiateGroupKeyShare(const std::string& target_node_id, const std::vector<uint8_t>& temp_key) {
    if (!isInitialized) {
        ESP_LOGE(TAG, "Security Manager not initialized.");
        return false;
    }

    ESP_LOGI(TAG, "Initiating group key share with node %s", target_node_id.c_str());

    // TODO:
    // 1. Create a GroupKeyShare protobuf message and populate it with the current groupKey.
    // 2. Serialize this protobuf message to a byte vector (plaintext).
    // 3. Encrypt this plaintext using the provided `temp_key`. This will produce an EncryptedPacket.
    // 4. Create a new AirComPacket.
    // 5. Set the payload of the AirComPacket to be the EncryptedPacket from the previous step.
    // 6. Set the `to_node` field to `target_node_id`.
    // 7. Serialize the final AirComPacket.
    // 8. Queue the final packet for sending via the network task's queue.

    ESP_LOGW(TAG, "Key share initiation is a placeholder!");
    return true;
}

bool SecurityManager::processGroupKeyShare(const EncryptedPacket& encrypted_packet, const std::vector<uint8_t>& temp_key) {
    if (!isInitialized) {
        ESP_LOGE(TAG, "Security Manager not initialized.");
        return false;
    }

    ESP_LOGI(TAG, "Processing incoming group key share packet.");

    // TODO:
    // 1. Decrypt the `encrypted_packet` using the provided `temp_key`. This will yield a plaintext byte vector.
    // 2. Deserialize the plaintext into a GroupKeyShare protobuf message.
    // 3. Extract the new group key from the message.
    // 4. Call `setGroupKey()` to update the group key for this session.
    // 5. Securely erase the temporary key and the plaintext key data from memory.

    ESP_LOGW(TAG, "Key share processing is a placeholder!");
    return true;
}
