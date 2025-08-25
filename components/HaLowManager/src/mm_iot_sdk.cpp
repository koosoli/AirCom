#include "../include/mm_iot_sdk.h"
#include <iostream>
#include <cstring>

// Tag for logging
static const char* TAG = "MM_IOT_SDK";

// Simple logging macros for when ESP-IDF is not available
#define ESP_LOGI(tag, format, ...) printf("[%s] " format "\n", tag, ##__VA_ARGS__)
#define ESP_LOGE(tag, format, ...) fprintf(stderr, "[%s ERROR] " format "\n", tag, ##__VA_ARGS__)

MMIoTSDK::MMIoTSDK()
    : m_initialized(false)
    , m_connected(false) {
    // Constructor - initialize member variables
}

MMIoTSDK::~MMIoTSDK() {
    // Destructor - cleanup if needed
    if (m_initialized) {
        // Cleanup resources
        m_initialized = false;
    }
}

bool MMIoTSDK::initialize(const std::string& ssid, const std::string& password, const std::string& country_code) {
    ESP_LOGI(TAG, "Initializing MM-IoT-SDK with SSID: %s", ssid.c_str());

    // Basic validation
    if (ssid.empty() || password.empty()) {
        ESP_LOGE(TAG, "Invalid SSID or password");
        return false;
    }

    // Simulate initialization process
    // In a real implementation, this would initialize the Wi-Fi HaLow hardware
    m_initialized = true;
    m_connected = false; // Will be updated when connection is established

    ESP_LOGI(TAG, "MM-IoT-SDK initialized successfully");
    return true;
}

bool MMIoTSDK::connectToPeer(const std::string& peer_id) {
    if (!m_initialized) {
        ESP_LOGE(TAG, "SDK not initialized");
        return false;
    }

    ESP_LOGI(TAG, "Connecting to peer: %s", peer_id.c_str());

    // Simulate connection process
    // In a real implementation, this would establish a connection to the peer
    m_connected = true;

    // Trigger connection callback if set
    if (m_connectionCallback) {
        m_connectionCallback(peer_id, true);
    }

    return true;
}

bool MMIoTSDK::disconnectFromPeer(const std::string& peer_id) {
    if (!m_initialized) {
        ESP_LOGE(TAG, "SDK not initialized");
        return false;
    }

    ESP_LOGI(TAG, "Disconnecting from peer: %s", peer_id.c_str());

    // Simulate disconnection process
    m_connected = false;

    // Trigger connection callback if set
    if (m_connectionCallback) {
        m_connectionCallback(peer_id, false);
    }

    return true;
}

bool MMIoTSDK::isConnected() const {
    return m_connected;
}

bool MMIoTSDK::sendData(const std::string& peer_id, const std::vector<uint8_t>& data) {
    if (!m_initialized || !m_connected) {
        ESP_LOGE(TAG, "Cannot send data: SDK not initialized or not connected");
        return false;
    }

    ESP_LOGI(TAG, "Sending %d bytes to peer: %s", data.size(), peer_id.c_str());

    // Simulate data transmission
    // In a real implementation, this would send the data over Wi-Fi HaLow

    // Trigger data callback if set (for received data simulation)
    if (m_dataCallback) {
        // This is just for testing - in reality, this would be triggered by incoming data
        // m_dataCallback(peer_id, data);
    }

    return true;
}

bool MMIoTSDK::broadcastData(const std::vector<uint8_t>& data) {
    if (!m_initialized || !m_connected) {
        ESP_LOGE(TAG, "Cannot broadcast data: SDK not initialized or not connected");
        return false;
    }

    ESP_LOGI(TAG, "Broadcasting %d bytes to all peers", data.size());

    // Simulate broadcast transmission
    // In a real implementation, this would broadcast the data to all peers

    return true;
}

bool MMIoTSDK::startDiscovery() {
    if (!m_initialized) {
        ESP_LOGE(TAG, "SDK not initialized");
        return false;
    }

    ESP_LOGI(TAG, "Starting peer discovery");

    // Simulate starting discovery process
    // In a real implementation, this would start discovering nearby peers

    // Add some dummy peers for testing
    m_discoveredPeers.push_back("peer_001");
    m_discoveredPeers.push_back("peer_002");
    m_discoveredPeers.push_back("peer_003");

    // Trigger discovery callback if set
    if (m_discoveryCallback) {
        m_discoveryCallback(m_discoveredPeers);
    }

    return true;
}

void MMIoTSDK::stopDiscovery() {
    if (!m_initialized) {
        ESP_LOGE(TAG, "SDK not initialized");
        return;
    }

    ESP_LOGI(TAG, "Stopping peer discovery");

    // Simulate stopping discovery process
    // In a real implementation, this would stop the discovery process
}

std::vector<std::string> MMIoTSDK::getDiscoveredPeers() {
    return m_discoveredPeers;
}

void MMIoTSDK::setConnectionCallback(std::function<void(const std::string&, bool)> callback) {
    m_connectionCallback = callback;
    ESP_LOGI(TAG, "Connection callback set");
}

void MMIoTSDK::setDataCallback(std::function<void(const std::string&, const std::vector<uint8_t>&)> callback) {
    m_dataCallback = callback;
    ESP_LOGI(TAG, "Data callback set");
}

void MMIoTSDK::setDiscoveryCallback(std::function<void(const std::vector<std::string>&)> callback) {
    m_discoveryCallback = callback;
    ESP_LOGI(TAG, "Discovery callback set");
}