#ifndef MM_IOT_SDK_H
#define MM_IOT_SDK_H

#include <string>
#include <vector>
#include <functional>

// Forward declarations for callback types
class MMIoTSDK {
public:
    MMIoTSDK();
    ~MMIoTSDK();

    // Initialization
    bool initialize(const std::string& ssid, const std::string& password, const std::string& country_code);

    // Connection management
    bool connectToPeer(const std::string& peer_id);
    bool disconnectFromPeer(const std::string& peer_id);
    bool isConnected() const;

    // Data transmission
    bool sendData(const std::string& peer_id, const std::vector<uint8_t>& data);
    bool broadcastData(const std::vector<uint8_t>& data);

    // Discovery
    bool startDiscovery();
    void stopDiscovery();
    std::vector<std::string> getDiscoveredPeers();

    // Callbacks
    void setConnectionCallback(std::function<void(const std::string&, bool)> callback);
    void setDataCallback(std::function<void(const std::string&, const std::vector<uint8_t>&)> callback);
    void setDiscoveryCallback(std::function<void(const std::vector<std::string>&)> callback);

private:
    bool m_initialized;
    bool m_connected;
    std::vector<std::string> m_discoveredPeers;

    // Callback functions
    std::function<void(const std::string&, bool)> m_connectionCallback;
    std::function<void(const std::string&, const std::vector<uint8_t>&)> m_dataCallback;
    std::function<void(const std::vector<std::string>&)> m_discoveryCallback;
};

#endif // MM_IOT_SDK_H