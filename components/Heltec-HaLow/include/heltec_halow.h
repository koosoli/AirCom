/**
 * @file heltec_halow.h
 * @brief Heltec ESP32 Wi-Fi HaLow implementation of IHaLow interface
 *
 * This file implements the IHaLow interface using Heltec's ESP32 Wi-Fi HaLow SDK,
 * providing Wi-Fi HaLow networking capabilities for Heltec hardware including HT-HC32 camera.
 *
 * @author AirCom Development Team
 * @version 2.0.0
 * @date 2024
 */

#ifndef HELTEC_HALOW_H
#define HELTEC_HALOW_H

#include "halow_interface.h"
#include <memory>

// Forward declarations for Heltec SDK types (to be defined when SDK is available)
struct heltec_halow_config_t;
struct heltec_network_info_t;
struct heltec_peer_info_t;
typedef void* heltec_handle_t;

/**
 * @brief Heltec ESP32 Wi-Fi HaLow implementation of IHaLow interface
 */
class HeltecHaLow : public IHaLow {
public:
    HeltecHaLow();
    ~HeltecHaLow() override;

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
    // Heltec SDK handle
    heltec_handle_t m_handle;

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

    // Camera-specific features (for HT-HC32)
    bool m_hasCamera;
    bool m_cameraInitialized;

    // Callback handlers
    void handleConnectionEvent(const std::string& peer_id, bool connected);
    void handleDataEvent(const std::string& peer_id, const std::vector<uint8_t>& data);
    void handleDiscoveryEvent(const std::vector<std::string>& peer_list);
    void handleCameraEvent(const std::string& event, void* data);

    // Helper methods
    HaLowPeerInfo convertPeerInfo(const std::string& peer_id) const;
    HaLowNetworkInfo convertNetworkInfo() const;
    bool validateConfig(const HaLowConfig& config) const;
    bool initializeCamera();
    bool configureHeltecPins();

    // Heltec-specific configurations
    bool configureHTHC32();
    bool configureHTIT01();
    bool configureGeneralHeltec();

    // Hardware-specific features
    uint32_t getHardwareFeatures() const;
    std::string getHardwareType() const;
};

/**
 * @brief Heltec-specific configuration extensions
 */
struct HeltecConfig : public HaLowConfig {
    // Camera configuration (for HT-HC32)
    bool enable_camera;
    int camera_resolution_width;
    int camera_resolution_height;
    int camera_frame_rate;
    int camera_quality;

    // Display configuration (if available)
    bool enable_display;
    int display_width;
    int display_height;

    // Power management
    bool enable_low_power_mode;
    bool enable_battery_monitoring;

    // Heltec-specific networking
    bool enable_mesh_optimization;
    bool enable_long_range_mode;
    int transmission_power_level;

    // Default constructor with Heltec-specific defaults
    HeltecConfig();
};

#endif // HELTEC_HALOW_H