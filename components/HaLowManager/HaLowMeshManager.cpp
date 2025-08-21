#include "include/HaLowMeshManager.h"
#include "esp_log.h"
#include "../../main/include/config.h" // Access config for TAG

// Private constructor for singleton
HaLowMeshManager::HaLowMeshManager() : isInitialized(false), isConnected(false) {
    // Constructor body
}

HaLowMeshManager::~HaLowMeshManager() {
    // Destructor body
}

bool HaLowMeshManager::begin() {
    ESP_LOGI(TAG, "Initializing HaLowMeshManager...");
    isInitialized = true;
    // In a real scenario, connection status would be updated by the SDK's events.
    // For this stub, we'll start as disconnected.
    isConnected = false;
    ESP_LOGI(TAG, "HaLowMeshManager initialized successfully (STUB).");
    return true;
}

void HaLowMeshManager::setConnectionStatus(bool status) {
    if (isConnected != status) {
        isConnected = status;
        ESP_LOGI(TAG, "Connection status changed to: %s", isConnected ? "Connected" : "Disconnected");
    }
}

bool HaLowMeshManager::get_connection_status() const {
    return isConnected;
}

bool HaLowMeshManager::sendUdpMulticast(const uint8_t* data, size_t size, uint16_t port) {
    if (!isInitialized) {
        ESP_LOGE(TAG, "Cannot send UDP, manager not initialized.");
        return false;
    }

    if (!isConnected) {
        ESP_LOGI(TAG, "Connection is down. Caching multicast message (%d bytes).", size);
        CachedMessage msg;
        msg.data.assign(data, data + size);
        msg.port = port;
        msg.isMulticast = true;
        messageCache.push_back(msg);
        return true; // Return true as the message is "handled" by caching it
    }

    ESP_LOGI(TAG, "Sending %d bytes via UDP multicast to port %d (STUB).", size, port);
    return true;
}

bool HaLowMeshManager::sendUdpUnicast(const std::string& destIp, const uint8_t* data, size_t size, uint16_t port) {
    if (!isInitialized) {
        ESP_LOGE(TAG, "Cannot send UDP, manager not initialized.");
        return false;
    }

    if (!isConnected) {
        ESP_LOGI(TAG, "Connection is down. Caching unicast message for %s (%d bytes).", destIp.c_str(), size);
        CachedMessage msg;
        msg.data.assign(data, data + size);
        msg.port = port;
        msg.destIp = destIp;
        msg.isMulticast = false;
        messageCache.push_back(msg);
        return true; // Return true as the message is "handled" by caching it
    }

    ESP_LOGI(TAG, "Sending %d bytes via UDP unicast to %s:%d (STUB).", size, destIp.c_str(), port);
    return true;
}

void HaLowMeshManager::sendCachedMessages() {
    if (messageCache.empty()) {
        return;
    }

    ESP_LOGI(TAG, "Connection restored. Sending %d cached messages...", messageCache.size());

    for (const auto& msg : messageCache) {
        if (msg.isMulticast) {
            ESP_LOGI(TAG, "Sending cached multicast message (%d bytes) to port %d.", msg.data.size(), msg.port);
            // Actually send it: sendUdpMulticast(&msg.data[0], msg.data.size(), msg.port);
        } else {
            ESP_LOGI(TAG, "Sending cached unicast message (%d bytes) to %s:%d.", msg.data.size(), msg.destIp.c_str(), msg.port);
            // Actually send it: sendUdpUnicast(msg.destIp, &msg.data[0], msg.data.size(), msg.port);
        }
    }

    messageCache.clear();
    ESP_LOGI(TAG, "Message cache cleared.");
}


std::vector<MeshNodeInfo> HaLowMeshManager::getMeshNodes() {
    std::vector<MeshNodeInfo> nodes;
    if (!isInitialized) {
        ESP_LOGE(TAG, "Cannot get nodes, manager not initialized.");
        return nodes;
    }

    ESP_LOGI(TAG, "Fetching mesh node list (STUB).");
    nodes.push_back({.macAddress = "DE:AD:BE:EF:00:01", .ipv6Address = "fe80::dead:beff:feef:1"});
    nodes.push_back({.macAddress = "DE:AD:BE:EF:00:02", .ipv6Address = "fe80::dead:beff:feef:2"});
    return nodes;
}
