/**
 * @file mm_iot_sdk.h
 * @brief MorseMicro MM-IoT-SDK wrapper for AirCom system
 *
 * This file provides a C++ wrapper around the MorseMicro MM-IoT-SDK
 * for Wi-Fi HaLow networking functionality.
 *
 * @author AirCom Development Team
 * @version 1.0.0
 * @date 2024
 */

#ifndef MM_IOT_SDK_H
#define MM_IOT_SDK_H

#include <cstdint>
#include <vector>
#include <string>
#include <functional>

// ESP-IDF includes
#include "esp_err.h"

// Forward declarations for MM-IoT-SDK types (to be defined when SDK is available)
struct mm_halow_config_t;
struct mm_network_info_t;
struct mm_peer_info_t;
typedef void* mm_handle_t;

// Callback function types
typedef std::function<void(const std::string& peer_id, bool connected)> ConnectionCallback;
typedef std::function<void(const std::string& peer_id, const std::vector<uint8_t>& data)> DataCallback;
typedef std::function<void(const std::vector<std::string>& peer_list)> DiscoveryCallback;

/**
 * @brief MM-IoT-SDK wrapper class for Wi-Fi HaLow functionality
 */
class MMIoTSDK {
public:
    // Singleton access
    static MMIoTSDK& getInstance() {
        static MMIoTSDK instance;
        return instance;
    }

    // Prevent copying
    MMIoTSDK(const MMIoTSDK&) = delete;
    void operator=(const MMIoTSDK&) = delete;

    /**
     * @brief Initialize the MM-IoT-SDK
     * @param ssid Network SSID for HaLow
     * @param password Network password
     * @param country_code Country code for regulatory compliance
     * @return true on success, false on failure
     */
    bool initialize(const std::string& ssid, const std::string& password, const std::string& country_code = "00");

    /**
     * @brief Deinitialize the MM-IoT-SDK
     */
    void deinitialize();

    /**
     * @brief Start network discovery
     * @return true on success, false on failure
     */
    bool startDiscovery();

    /**
     * @brief Stop network discovery
     */
    void stopDiscovery();

    /**
     * @brief Connect to a specific peer
     * @param peer_id Peer identifier to connect to
     * @return true on success, false on failure
     */
    bool connectToPeer(const std::string& peer_id);

    /**
     * @brief Disconnect from a specific peer
     * @param peer_id Peer identifier to disconnect from
     * @return true on success, false on failure
     */
    bool disconnectFromPeer(const std::string& peer_id);

    /**
     * @brief Send data to a specific peer
     * @param peer_id Target peer identifier
     * @param data Data to send
     * @return true on success, false on failure
     */
    bool sendData(const std::string& peer_id, const std::vector<uint8_t>& data);

    /**
     * @brief Broadcast data to all connected peers
     * @param data Data to broadcast
     * @return true on success, false on failure
     */
    bool broadcastData(const std::vector<uint8_t>& data);

    /**
     * @brief Get list of discovered peers
     * @return Vector of peer IDs
     */
    std::vector<std::string> getDiscoveredPeers();

    /**
     * @brief Get list of connected peers
     * @return Vector of peer IDs
     */
    std::vector<std::string> getConnectedPeers();

    /**
     * @brief Get network information
     * @return Network info structure
     */
    mm_network_info_t getNetworkInfo();

    /**
     * @brief Set connection callback
     * @param callback Function to call on connection events
     */
    void setConnectionCallback(ConnectionCallback callback);

    /**
     * @brief Set data callback
     * @param callback Function to call on data reception
     */
    void setDataCallback(DataCallback callback);

    /**
     * @brief Set discovery callback
     * @param callback Function to call on discovery events
     */
    void setDiscoveryCallback(DiscoveryCallback callback);

    /**
     * @brief Get SDK initialization status
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const { return m_initialized; }

    /**
     * @brief Get connection status
     * @return true if connected to network, false otherwise
     */
    bool isConnected() const { return m_connected; }

private:
    // Private constructor for singleton
    MMIoTSDK();
    ~MMIoTSDK();

    // MM-IoT-SDK internal state
    mm_handle_t m_handle;
    bool m_initialized;
    bool m_connected;
    bool m_discovering;

    // Configuration
    std::string m_ssid;
    std::string m_password;
    std::string m_country_code;

    // Callbacks
    ConnectionCallback m_connectionCallback;
    DataCallback m_dataCallback;
    DiscoveryCallback m_discoveryCallback;

    // Internal helper methods
    bool configureSPI();
    bool configurePins();
    esp_err_t initializeHardware();
    void handleConnectionEvent(const std::string& peer_id, bool connected);
    void handleDataEvent(const std::string& peer_id, const std::vector<uint8_t>& data);
    void handleDiscoveryEvent(const std::vector<std::string>& peer_list);

    // SPI configuration for FGH100M-H module
    static const int SPI_HOST = 1;
    static const int SPI_CLOCK_SPEED = 1000000; // 1 MHz
    static const int SPI_MODE = 0;

    // XIAO ESP32 pin assignments (from configuration file)
    static const int PIN_MOSI = get_spi_mosi_pin();
    static const int PIN_MISO = get_spi_miso_pin();
    static const int PIN_SCLK = get_spi_sclk_pin();
    static const int PIN_CS = get_spi_cs_pin();
    static const int PIN_RESET = get_spi_reset_pin();
    static const int PIN_INT = get_spi_int_pin();
};

/**
 * @brief MM-IoT-SDK configuration structure
 */
struct MMIoTConfig {
    std::string ssid;
    std::string password;
    std::string country_code;
    uint32_t channel;
    uint32_t bandwidth;
    bool enable_mesh;
    uint32_t max_connections;
    uint32_t heartbeat_interval;
};

/**
 * @brief Network information structure
 */
struct MMIoTNetworkInfo {
    std::string network_id;
    std::string device_id;
    uint32_t channel;
    uint32_t bandwidth;
    int32_t rssi;
    uint32_t connected_peers;
    bool mesh_enabled;
};

#endif // MM_IOT_SDK_H