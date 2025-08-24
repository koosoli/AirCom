#ifndef HALOW_MESH_MANAGER_H
#define HALOW_MESH_MANAGER_H

#include <cstdint>
#include <vector>
#include <string>
#include <memory>

// Enum to define the power management roles for a node
enum class PowerRole {
    ROUTER,          // Always on, routes traffic for other nodes.
    SLEEPY_END_DEVICE // Can enter low-power sleep modes.
};

// Forward declaration for MM-IoT-SDK
class MMIoTSDK;

// Forward declaration for safe callback system
class CallbackOwner;
class ConnectionCallback;
class DataCallback;
class DiscoveryCallback;

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

    // Start network discovery
    bool startDiscovery();

    // Stop network discovery
    void stopDiscovery();

    // Connect to a specific peer
    bool connectToPeer(const std::string& peer_id);

    // Disconnect from a specific peer
    bool disconnectFromPeer(const std::string& peer_id);

    // Set the power management role for this device
    void setPowerRole(PowerRole newRole);

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

    // MM-IoT-SDK instance
    MMIoTSDK* m_mmSDK;

    // Safe callback system
    CallbackOwner m_callbackOwner;
    std::shared_ptr<ConnectionCallback> m_connectionCallback;
    std::shared_ptr<DataCallback> m_dataCallback;
    std::shared_ptr<DiscoveryCallback> m_discoveryCallback;

    // Callback handlers for MM-IoT-SDK events
    void handleConnectionEvent(const std::string& peer_id, bool connected);
    void handleDataEvent(const std::string& peer_id, const std::vector<uint8_t>& data);
    void handleDiscoveryEvent(const std::vector<std::string>& peer_list);

    // Apply the current power role settings to the hardware
    void applyPowerRole();

    // The current power management role of the device
    PowerRole currentPowerRole;
};

#endif // HALOW_MESH_MANAGER_H
