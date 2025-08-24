#ifndef SECURITY_MANAGER_H
#define SECURITY_MANAGER_H

#include <vector>
#include <string>
#include <memory>

// Forward declare the protobuf message
class EncryptedPacket;

class SecurityManager {
public:
    // Singleton access method
    static SecurityManager& getInstance() {
        static SecurityManager instance;
        return instance;
    }

    // Deleted copy constructor and assignment operator for singleton
    SecurityManager(const SecurityManager&) = delete;
    void operator=(const SecurityManager&) = delete;

    /**
     * @brief Initializes the security manager and underlying crypto library.
     * @return True if initialization is successful, false otherwise.
     */
    bool begin();

    /**
     * @brief Encrypts a plaintext payload.
     * @param plaintext The data to encrypt.
     * @param encrypted_packet The protobuf message to populate with the nonce and ciphertext.
     * @return True on success, false on failure.
     */
    bool encrypt(const std::vector<uint8_t>& plaintext, EncryptedPacket& encrypted_packet);

    /**
     * @brief Decrypts an EncryptedPacket.
     * @param encrypted_packet The protobuf message containing the nonce and ciphertext.
     * @param plaintext A vector that will be populated with the decrypted data.
     * @return True on success (i.e., authentication tag is valid), false otherwise.
     */
    bool decrypt(const EncryptedPacket& encrypted_packet, std::vector<uint8_t>& plaintext);

    /**
     * @brief Updates the group key used for session-wide encryption.
     * @param new_key The new group key.
     */
    void setGroupKey(const std::vector<uint8_t>& new_key);

    /**
     * @brief Initiates a key share with a target node.
     * @param target_node_id The ID of the node to share the key with.
     * @param temp_key A pre-shared temporary key for this specific exchange.
     * @return True if the key share message was successfully created and queued for sending.
     */
    bool initiateGroupKeyShare(const std::string& target_node_id, const std::vector<uint8_t>& temp_key);

    /**
     * @brief Processes an incoming key share packet.
     * @param encrypted_packet The encrypted packet containing the GroupKeyShare message.
     * @param temp_key The pre-shared temporary key to decrypt this specific message.
     * @return True if the key was successfully decrypted and updated.
     */
    bool processGroupKeyShare(const EncryptedPacket& encrypted_packet, const std::vector<uint8_t>& temp_key);

private:
    // Private constructor for singleton
    SecurityManager();
    ~SecurityManager();

    bool isInitialized;

    // The primary key used for encrypting and decrypting group messages.
    // In a real implementation, this would be managed by the key exchange protocol.
    std::vector<uint8_t> groupKey;
};

#endif // SECURITY_MANAGER_H
