#ifndef HALOW_MESH_MANAGER_H
#define HALOW_MESH_MANAGER_H

#include <cstdint>
#include <vector>
#include <string>

// Forward declaration for a potential node info structure
struct MeshNodeInfo {
    std::string macAddress;
    std::string ipv6Address;
    // Add other relevant info like RSSI, etc.
};

// Structure to hold a cached message
struct CachedMessage {
    std::vector<uint8_t> data;
    uint16_t port;
    std::string destIp; // Empty for multicast
    bool isMulticast;
};


class HaLowMeshManager {
public:
    // Singleton access method
    static HaLowMeshManager& getInstance() {
        static HaLowMeshManager instance;
        return instance;
    }

    // Deleted copy constructor and assignment operator for singleton
    HaLowMeshManager(const HaLowMeshManager&) = delete;
    void operator=(const HaLowMeshManager&) = delete;

    // Initialize the HaLow radio and SDK
    bool begin();

    // Send a UDP packet to a multicast address
    bool sendUdpMulticast(const uint8_t* data, size_t size, uint16_t port);

    // Send a UDP packet to a specific unicast address
    bool sendUdpUnicast(const std::string& destIp, const uint8_t* data, size_t size, uint16_t port);

    // Get a list of discovered mesh nodes
    std::vector<MeshNodeInfo> getMeshNodes();

    // Connection status management
    void setConnectionStatus(bool status);
    bool get_connection_status() const;

    // Send any messages that were cached while offline
    void sendCachedMessages();


private:
    // Private constructor for singleton
    HaLowMeshManager();
    ~HaLowMeshManager();

    // Flag to track initialization status
    bool isInitialized;
    // Flag to track mesh connection status
    bool isConnected;

    // Cache for messages to be sent when connection is restored
    std::vector<CachedMessage> messageCache;
};

#endif // HALOW_MESH_MANAGER_H
