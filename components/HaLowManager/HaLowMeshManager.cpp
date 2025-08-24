#include "include/HaLowMeshManager.h"
#include "esp_log.h"
#include "../../main/include/config.h" // Access config for TAG
#include "../../main/include/safe_callback.h" // Safe callback system
#include "mm_iot_sdk.h" // Include the MM-IoT-SDK wrapper

// Private constructor for singleton
HaLowMeshManager::HaLowMeshManager()
    : isInitialized(false)
    , isConnected(false)
    , m_mmSDK(nullptr)
    , m_callbackOwner("HaLowMeshManager") {
    // Constructor body
}

HaLowMeshManager::~HaLowMeshManager() {
    // Destructor body
    if (m_mmSDK) {
        delete m_mmSDK;
        m_mmSDK = nullptr;
    }
}

bool HaLowMeshManager::begin() {
    ESP_LOGI(TAG, "Initializing HaLowMeshManager with MM-IoT-SDK...");

    // Initialize MM-IoT-SDK
    m_mmSDK = new MMIoTSDK();

    // Get configuration from config.h
    std::string ssid = "AirCom-HaLow";
    std::string password = "aircom2024";
    std::string country_code = "00"; // Default country code

    if (m_mmSDK->initialize(ssid, password, country_code)) {
        isInitialized = true;
        isConnected = false; // Will be updated by callbacks

        // Set up safe callbacks using the callback manager
        m_connectionCallback = createConnectionCallback(
            [this](const std::string& peer_id, bool connected) {
                this->handleConnectionEvent(peer_id, connected);
            },
            m_callbackOwner.getOwnerId()
        );

        m_dataCallback = createDataCallback(
            [this](const std::string& peer_id, const std::vector<uint8_t>& data) {
                this->handleDataEvent(peer_id, data);
            },
            m_callbackOwner.getOwnerId()
        );

        m_discoveryCallback = createDiscoveryCallback(
            [this](const std::vector<std::string>& peer_list) {
                this->handleDiscoveryEvent(peer_list);
            },
            m_callbackOwner.getOwnerId()
        );

        // Register callbacks with MM-IoT-SDK (using raw function pointers for compatibility)
        m_mmSDK->setConnectionCallback([this](const std::string& peer_id, bool connected) {
            if (m_connectionCallback && m_connectionCallback->isValid()) {
                m_connectionCallback->execute(peer_id, connected);
            }
        });

        m_mmSDK->setDataCallback([this](const std::string& peer_id, const std::vector<uint8_t>& data) {
            if (m_dataCallback && m_dataCallback->isValid()) {
                m_dataCallback->execute(peer_id, data);
            }
        });

        m_mmSDK->setDiscoveryCallback([this](const std::vector<std::string>& peer_list) {
            if (m_discoveryCallback && m_discoveryCallback->isValid()) {
                m_discoveryCallback->execute(peer_list);
            }
        });

        ESP_LOGI(TAG, "HaLowMeshManager initialized successfully with safe callback system.");
        return true;
    } else {
        ESP_LOGE(TAG, "Failed to initialize MM-IoT-SDK");
        delete m_mmSDK;
        m_mmSDK = nullptr;
        return false;
    }
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
    if (!isInitialized || !m_mmSDK) {
        ESP_LOGE(TAG, "Cannot send UDP multicast, manager not initialized.");
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

    // Use MM-IoT-SDK to broadcast data
    std::vector<uint8_t> dataVec(data, data + size);
    bool success = m_mmSDK->broadcastData(dataVec);

    if (success) {
        ESP_LOGI(TAG, "Sent %d bytes via MM-IoT-SDK broadcast", size);
    } else {
        ESP_LOGE(TAG, "Failed to send multicast via MM-IoT-SDK");
    }

    return success;
}

bool HaLowMeshManager::sendUdpUnicast(const std::string& destIp, const uint8_t* data, size_t size, uint16_t port) {
    if (!isInitialized || !m_mmSDK) {
        ESP_LOGE(TAG, "Cannot send UDP unicast, manager not initialized.");
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

    // Use MM-IoT-SDK to send unicast data
    std::vector<uint8_t> dataVec(data, data + size);
    bool success = m_mmSDK->sendData(destIp, dataVec);

    if (success) {
        ESP_LOGI(TAG, "Sent %d bytes via MM-IoT-SDK unicast to %s", size, destIp.c_str());
    } else {
        ESP_LOGE(TAG, "Failed to send unicast via MM-IoT-SDK to %s", destIp.c_str());
    }

    return success;
}

void HaLowMeshManager::sendCachedMessages() {
    if (messageCache.empty()) {
        return;
    }

    ESP_LOGI(TAG, "Connection restored. Sending %d cached messages...", messageCache.size());

    for (const auto& msg : messageCache) {
        if (msg.isMulticast) {
            ESP_LOGI(TAG, "Sending cached multicast message (%d bytes) to port %d.", msg.data.size(), msg.port);
            sendUdpMulticast(msg.data.data(), msg.data.size(), msg.port);
        } else {
            ESP_LOGI(TAG, "Sending cached unicast message (%d bytes) to %s:%d.", msg.data.size(), msg.destIp.c_str(), msg.port);
            sendUdpUnicast(msg.destIp, msg.data.data(), msg.data.size(), msg.port);
        }
    }

    messageCache.clear();
    ESP_LOGI(TAG, "Message cache cleared.");
}

std::vector<MeshNodeInfo> HaLowMeshManager::getMeshNodes() {
    std::vector<MeshNodeInfo> nodes;
    if (!isInitialized || !m_mmSDK) {
        ESP_LOGE(TAG, "Cannot get nodes, manager not initialized.");
        return nodes;
    }

    // Get discovered peers from MM-IoT-SDK
    auto peers = m_mmSDK->getDiscoveredPeers();

    // Convert to MeshNodeInfo format
    for (const auto& peer : peers) {
        MeshNodeInfo node;
        node.macAddress = "MM:IO:T-" + peer.substr(0, 8); // Generate MAC-like address from peer ID
        node.ipv6Address = "fe80::mmiot:" + peer.substr(0, 4) + ":" + peer.substr(4, 4);
        nodes.push_back(node);
    }

    ESP_LOGI(TAG, "Fetched %d mesh nodes from MM-IoT-SDK", nodes.size());
    return nodes;
}

// Additional methods for MM-IoT-SDK integration
bool HaLowMeshManager::startDiscovery() {
    if (!isInitialized || !m_mmSDK) {
        ESP_LOGE(TAG, "Cannot start discovery, manager not initialized.");
        return false;
    }

    return m_mmSDK->startDiscovery();
}

void HaLowMeshManager::stopDiscovery() {
    if (!isInitialized || !m_mmSDK) {
        ESP_LOGE(TAG, "Cannot stop discovery, manager not initialized.");
        return;
    }

    m_mmSDK->stopDiscovery();
}

bool HaLowMeshManager::connectToPeer(const std::string& peer_id) {
    if (!isInitialized || !m_mmSDK) {
        ESP_LOGE(TAG, "Cannot connect to peer, manager not initialized.");
        return false;
    }

    return m_mmSDK->connectToPeer(peer_id);
}

bool HaLowMeshManager::disconnectFromPeer(const std::string& peer_id) {
    if (!isInitialized || !m_mmSDK) {
        ESP_LOGE(TAG, "Cannot disconnect from peer, manager not initialized.");
        return false;
    }

    return m_mmSDK->disconnectFromPeer(peer_id);
}

// Callback handlers for MM-IoT-SDK events
void HaLowMeshManager::handleConnectionEvent(const std::string& peer_id, bool connected) {
    ESP_LOGI(TAG, "MM-IoT-SDK connection event: %s %s",
             peer_id.c_str(), connected ? "connected" : "disconnected");

    setConnectionStatus(connected);

    if (connected) {
        // Send any cached messages now that we're connected
        sendCachedMessages();

        // Start discovery if not already started
        if (m_mmSDK && !m_mmSDK->isInitialized()) {
            m_mmSDK->startDiscovery();
        }
    }
}

void HaLowMeshManager::handleDataEvent(const std::string& peer_id, const std::vector<uint8_t>& data) {
    ESP_LOGI(TAG, "MM-IoT-SDK data event: %d bytes from %s", data.size(), peer_id.c_str());

    // Handle incoming data - this could be extended to integrate with the main message queue
    // For now, just log the event
}

void HaLowMeshManager::handleDiscoveryEvent(const std::vector<std::string>& peer_list) {
    ESP_LOGI(TAG, "MM-IoT-SDK discovery event: found %d peers", peer_list.size());

    // Discovery event handling - could update peer lists, etc.
    for (const auto& peer : peer_list) {
        ESP_LOGD(TAG, "Discovered peer: %s", peer.c_str());
    }
}
