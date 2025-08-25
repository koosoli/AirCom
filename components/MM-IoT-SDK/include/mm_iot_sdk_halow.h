/**
 * @file mm_iot_sdk_halow.h
 * @brief MM-IoT-SDK implementation of IHaLow interface
 *
 * This file implements the IHaLow interface using MorseMicro's MM-IoT-SDK,
 * providing Wi-Fi HaLow networking capabilities for supported hardware.
 *
 * @author AirCom Development Team
 * @version 2.0.0
 * @date 2024
 */

#ifndef MM_IOT_SDK_HALOW_H
#define MM_IOT_SDK_HALOW_H

#include "halow_interface.h"
#include "mm_iot_sdk.h"
#include <memory>

/**
 * @brief MM-IoT-SDK implementation of IHaLow interface
 */
class MMIoTSDKHaLow : public IHaLow {
public:
    MMIoTSDKHaLow();
    ~MMIoTSDKHaLow() override;

    // IHaLow interface implementation
    bool initialize(const HaLowConfig& config) override;
    void deinitialize() override;
    bool startDiscovery() override;
    void stopDiscovery() override;
    bool connectToPeer(const std::string& peer_id) override;
    bool disconnectFromPeer(const std::string& peer_id) override;
    bool sendData(const std::string& peer_id, const std::vector<uint8_t>& data) override;
    bool broadcastData(const std::vector<uint8_t>& data) override;
    std::vector<HaLowPeerInfo> getDiscoveredPeers() override;
    std::vector<HaLowPeerInfo> getConnectedPeers() override;
    HaLowNetworkInfo getNetworkInfo() override;
    void setConnectionCallback(ConnectionCallback callback) override;
    void setDataCallback(DataCallback callback) override;
    void setDiscoveryCallback(DiscoveryCallback callback) override;
    void setEventCallback(EventCallback callback) override;
    std::string getImplementationName() const override;
    std::vector<std::string> getSupportedHardware() const override;
    bool isInitialized() const override;
    bool isConnected() const override;
    std::string getVersion() const override;
    std::string sendRawCommand(const std::string& command, const std::vector<std::string>& params) override;

private:
    // MM-IoT-SDK instance
    std::unique_ptr<MMIoTSDK> m_mmSDK;

    // Configuration
    HaLowConfig m_config;

    // Callbacks
    ConnectionCallback m_connectionCallback;
    DataCallback m_dataCallback;
    DiscoveryCallback m_discoveryCallback;
    EventCallback m_eventCallback;

    // Internal state
    bool m_initialized;
    bool m_discovering;

    // Callback handlers
    void handleConnectionEvent(const std::string& peer_id, bool connected);
    void handleDataEvent(const std::string& peer_id, const std::vector<uint8_t>& data);
    void handleDiscoveryEvent(const std::vector<std::string>& peer_list);

    // Helper methods
    HaLowPeerInfo convertPeerInfo(const std::string& peer_id) const;
    HaLowNetworkInfo convertNetworkInfo() const;
    bool validateConfig(const HaLowConfig& config) const;
};

#endif // MM_IOT_SDK_HALOW_H