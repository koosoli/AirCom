/**
 * @file halow_interface.h
 * @brief Abstract interface for Wi-Fi HaLow networking implementations
 *
 * This file provides a unified interface for different Wi-Fi HaLow implementations,
 * allowing the AirCom system to support multiple hardware platforms and SDKs.
 *
 * @author AirCom Development Team
 * @version 2.0.0
 * @date 2024
 */

#ifndef HALOW_INTERFACE_H
#define HALOW_INTERFACE_H

#include <cstdint>
#include <vector>
#include <string>
#include <functional>
#include <memory>

// Forward declarations
struct HaLowNetworkInfo;
struct HaLowPeerInfo;
struct HaLowConfig;

// Callback function types
typedef std::function<void(const std::string& peer_id, bool connected)> ConnectionCallback;
typedef std::function<void(const std::string& peer_id, const std::vector<uint8_t>& data)> DataCallback;
typedef std::function<void(const std::vector<std::string>& peer_list)> DiscoveryCallback;
typedef std::function<void(const std::string& event, void* data)> EventCallback;

/**
 * @brief Wi-Fi HaLow network information structure
 */
struct HaLowNetworkInfo {
    std::string network_id;
    std::string device_id;
    uint32_t channel;
    uint32_t bandwidth;
    int32_t rssi;
    uint32_t connected_peers;
    bool mesh_enabled;
    std::string sdk_version;
    std::string hardware_type;
};

/**
 * @brief Wi-Fi HaLow peer information structure
 */
struct HaLowPeerInfo {
    std::string peer_id;
    std::string mac_address;
    std::string ipv6_address;
    int32_t rssi;
    uint32_t connection_time;
    bool is_connected;
    std::string device_type;
};

/**
 * @brief Wi-Fi HaLow configuration structure
 */
struct HaLowConfig {
    std::string ssid;
    std::string password;
    std::string country_code;
    uint32_t channel;
    uint32_t bandwidth;
    bool enable_mesh;
    uint32_t max_connections;
    uint32_t heartbeat_interval;
    uint32_t discovery_timeout;
    bool enable_encryption;
    std::string encryption_key;
    std::vector<std::string> allowed_peer_ids;
};

/**
 * @brief Abstract base class for Wi-Fi HaLow implementations
 */
class IHaLow {
public:
    IHaLow() = default;
    virtual ~IHaLow() = default;

    // Prevent copying
    IHaLow(const IHaLow&) = delete;
    IHaLow& operator=(const IHaLow&) = delete;

    /**
     * @brief Initialize the Wi-Fi HaLow implementation
     * @param config Configuration parameters
     * @return true on success, false on failure
     */
    virtual bool initialize(const HaLowConfig& config) = 0;

    /**
     * @brief Deinitialize the Wi-Fi HaLow implementation
     */
    virtual void deinitialize() = 0;

    /**
     * @brief Start network discovery
     * @return true on success, false on failure
     */
    virtual bool startDiscovery() = 0;

    /**
     * @brief Stop network discovery
     */
    virtual void stopDiscovery() = 0;

    /**
     * @brief Connect to a specific peer
     * @param peer_id Peer identifier to connect to
     * @return true on success, false on failure
     */
    virtual bool connectToPeer(const std::string& peer_id) = 0;

    /**
     * @brief Disconnect from a specific peer
     * @param peer_id Peer identifier to disconnect from
     * @return true on success, false on failure
     */
    virtual bool disconnectFromPeer(const std::string& peer_id) = 0;

    /**
     * @brief Send data to a specific peer
     * @param peer_id Target peer identifier
     * @param data Data to send
     * @return true on success, false on failure
     */
    virtual bool sendData(const std::string& peer_id, const std::vector<uint8_t>& data) = 0;

    /**
     * @brief Broadcast data to all connected peers
     * @param data Data to broadcast
     * @return true on success, false on failure
     */
    virtual bool broadcastData(const std::vector<uint8_t>& data) = 0;

    /**
     * @brief Get list of discovered peers
     * @return Vector of peer information
     */
    virtual std::vector<HaLowPeerInfo> getDiscoveredPeers() = 0;

    /**
     * @brief Get list of connected peers
     * @return Vector of peer information
     */
    virtual std::vector<HaLowPeerInfo> getConnectedPeers() = 0;

    /**
     * @brief Get network information
     * @return Network information structure
     */
    virtual HaLowNetworkInfo getNetworkInfo() = 0;

    /**
     * @brief Set connection callback
     * @param callback Function to call on connection events
     */
    virtual void setConnectionCallback(ConnectionCallback callback) = 0;

    /**
     * @brief Set data callback
     * @param callback Function to call on data reception
     */
    virtual void setDataCallback(DataCallback callback) = 0;

    /**
     * @brief Set discovery callback
     * @param callback Function to call on discovery events
     */
    virtual void setDiscoveryCallback(DiscoveryCallback callback) = 0;

    /**
     * @brief Set general event callback
     * @param callback Function to call on general events
     */
    virtual void setEventCallback(EventCallback callback) = 0;

    /**
     * @brief Get implementation name
     * @return Implementation identifier
     */
    virtual std::string getImplementationName() const = 0;

    /**
     * @brief Get supported hardware types
     * @return Vector of supported hardware identifiers
     */
    virtual std::vector<std::string> getSupportedHardware() const = 0;

    /**
     * @brief Check if implementation is initialized
     * @return true if initialized, false otherwise
     */
    virtual bool isInitialized() const = 0;

    /**
     * @brief Check if connected to network
     * @return true if connected, false otherwise
     */
    virtual bool isConnected() const = 0;

    /**
     * @brief Get implementation version
     * @return Version string
     */
    virtual std::string getVersion() const = 0;

    /**
     * @brief Send raw command to implementation (for debugging)
     * @param command Command string
     * @param params Command parameters
     * @return Response string
     */
    virtual std::string sendRawCommand(const std::string& command, const std::vector<std::string>& params) = 0;
};

/**
 * @brief Factory class for creating Wi-Fi HaLow implementations
 */
class HaLowFactory {
public:
    /**
     * @brief Create Wi-Fi HaLow implementation based on hardware and preferences
     * @param hardware_type Target hardware type
     * @param preferred_sdk Preferred SDK (optional)
     * @return Unique pointer to HaLow implementation
     */
    static std::unique_ptr<IHaLow> createHaLow(const std::string& hardware_type,
                                             const std::string& preferred_sdk = "");

    /**
     * @brief Get list of available implementations
     * @return Vector of implementation names
     */
    static std::vector<std::string> getAvailableImplementations();

    /**
     * @brief Check if hardware is supported by any implementation
     * @param hardware_type Hardware type to check
     * @return true if supported, false otherwise
     */
    static bool isHardwareSupported(const std::string& hardware_type);

    /**
     * @brief Get supported implementations for specific hardware
     * @param hardware_type Hardware type
     * @return Vector of implementation names
     */
    static std::vector<std::string> getSupportedImplementations(const std::string& hardware_type);
};

/**
 * @brief Hardware abstraction layer for different ESP32 boards
 */
class HardwareAbstraction {
public:
    /**
     * @brief Detect current hardware platform
     * @return Hardware type string
     */
    static std::string detectHardware();

    /**
     * @brief Get pin configuration for specific hardware
     * @param hardware_type Hardware type
     * @return Pin configuration structure
     */
    static PinConfig getPinConfig(const std::string& hardware_type);

    /**
     * @brief Check if hardware is supported
     * @param hardware_type Hardware type
     * @return true if supported, false otherwise
     */
    static bool isHardwareSupported(const std::string& hardware_type);

    /**
     * @brief Get hardware-specific features
     * @param hardware_type Hardware type
     * @return Feature flags
     */
    static uint32_t getHardwareFeatures(const std::string& hardware_type);
};

/**
 * @brief Pin configuration structure
 */
struct PinConfig {
    int spi_mosi;
    int spi_miso;
    int spi_sclk;
    int spi_cs;
    int spi_reset;
    int spi_int;
    int uart_tx;
    int uart_rx;
    int i2c_sda;
    int i2c_scl;
    int led_pin;
    int button_pin;
    int battery_adc;
    int sd_cs;
    int camera_d0;
    int camera_d1;
    int camera_d2;
    int camera_d3;
    int camera_d4;
    int camera_d5;
    int camera_d6;
    int camera_d7;
    int camera_pclk;
    int camera_href;
    int camera_vsync;
    int camera_xclk;
    int camera_sda;
    int camera_scl;
};

/**
 * @brief Feature flags for hardware capabilities
 */
enum HardwareFeatures {
    FEATURE_WIFI_HALOW    = (1 << 0),
    FEATURE_CAMERA        = (1 << 1),
    FEATURE_DISPLAY       = (1 << 2),
    FEATURE_GPS           = (1 << 3),
    FEATURE_AUDIO         = (1 << 4),
    FEATURE_SD_CARD       = (1 << 5),
    FEATURE_BATTERY_MONITOR = (1 << 6),
    FEATURE_LOW_POWER     = (1 << 7),
    FEATURE_DUAL_CORE     = (1 << 8),
    FEATURE_BLUETOOTH     = (1 << 9),
    FEATURE_ETHERNET      = (1 << 10),
    FEATURE_USB           = (1 << 11)
};

#endif // HALOW_INTERFACE_H