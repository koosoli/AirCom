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

    // TODO: Add methods for TCP communication
    // bool connectTcp(const std::string& destIp, uint16_t port);
    // bool sendTcp(const uint8_t* data, size_t size);
    // void disconnectTcp();

    // Get a list of discovered mesh nodes
    std::vector<MeshNodeInfo> getMeshNodes();

private:
    // Private constructor for singleton
    HaLowMeshManager();
    ~HaLowMeshManager();

    // Flag to track initialization status
    bool isInitialized;
};

#endif // HALOW_MESH_MANAGER_H
