/**
 * @file network_task.cpp
 * @brief Network task implementation for AirCom system
 *
 * This file contains the network task functionality extracted from main.cpp
 * for better modularity and maintainability.
 *
 * @author AirCom Development Team
 * @version 1.0.0
 * @date 2024
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "include/config.h"
#include "include/shared_data.h"
#include "include/network_utils.h"
#include "include/error_handling.h"
#include "include/crypto.h"
#include "sdkconfig.h" // For Kconfig options
#if CONFIG_AIRCOM_FEATURE_SECURITY
#include "include/SecurityManager.h"
#endif


// Define mutex timeout constants locally (should be in shared_data.h)
#define MUTEX_TIMEOUT_DEFAULT pdMS_TO_TICKS(500)
#include "HaLowMeshManager.h"
#include "AirCom.pb-c.h"

static const char* TAG = "NETWORK_TASK";

// ============================================================================
// INCOMING PACKET HANDLER
// ============================================================================

/**
 * @brief Processes any incoming AirCom packet.
 * @param data Raw byte buffer of the packet.
 * @param len Length of the buffer.
 */
static void handle_incoming_packet(const uint8_t* data, size_t len) {
    AirComPacket *packet = air_com_packet__unpack(NULL, len, data);
    if (!packet) {
        ESP_LOGE(TAG, "Failed to unpack incoming protobuf packet");
        return;
    }

    ESP_LOGI(TAG, "Processing packet from %s", packet->from_node);

    switch (packet->payload_variant_case) {
        case AIR_COM_PACKET__PAYLOAD_VARIANT_NODE_INFO:
            // TODO: Update contact list based on NodeInfo
            break;

#if CONFIG_AIRCOM_FEATURE_SECURITY
        case AIR_COM_PACKET__PAYLOAD_VARIANT_ENCRYPTED_PACKET:
        {
            ESP_LOGI(TAG, "Received an encrypted packet.");
            std::vector<uint8_t> plaintext;
            // For now, we assume a hardcoded temporary key for key share packets.
            // A real implementation would need a more robust way to manage temp keys.
            std::vector<uint8_t> temp_key(32, 'A');

            // First, try to decrypt it as a key share packet.
            if (SecurityManager::getInstance().processGroupKeyShare(*packet->encrypted_packet, temp_key)) {
                ESP_LOGI(TAG, "Successfully processed group key share.");
            } else {
                // If not a key share, try to decrypt as a normal message.
                if (SecurityManager::getInstance().decrypt(*packet->encrypted_packet, plaintext)) {
                    // TODO: Once decrypted, the plaintext is another AirComPacket.
                    // We would need to deserialize it and handle its content (e.g., TextMessage).
                    ESP_LOGI(TAG, "Successfully decrypted application packet.");
                } else {
                    ESP_LOGW(TAG, "Failed to decrypt packet from %s. Discarding.", packet->from_node);
                }
            }
            break;
        }
#endif

        // Note: TextMessage and cot_message are now inside EncryptedPacket,
        // so they are not handled here directly anymore.

        default:
            ESP_LOGW(TAG, "Received unhandled packet type: %d", packet->payload_variant_case);
            break;
    }

    air_com_packet__free_unpacked(packet, NULL);
}


// ============================================================================
// NETWORK TASK IMPLEMENTATION
// ============================================================================

/**
 * @brief Network discovery and communication task
 *
 * This task handles network discovery, mesh management, and communication
 * with other AirCom nodes.
 */
void networkTask(void *pvParameters) {
    ESP_LOGI(TAG, "networkTask started");

    // Initialize network utilities
    if (network_utils_init() != true) {
        ESP_LOGE(TAG, "Failed to initialize network utilities");
        vTaskDelete(NULL);
        return;
    }

    // Initialize the HaLow Mesh Manager
    HaLowMeshManager& meshManager = HaLowMeshManager::getInstance();
    meshManager.begin();

    // Main task loop
    for (;;) {
        ESP_LOGI(TAG, "Broadcasting discovery packet...");

        // 1. Create a NodeInfo packet to broadcast our presence.
        AirComPacket packet = AIR_COM_PACKET__INIT;
        NodeInfo node_info = NODE_INFO__INIT;

        packet.payload_variant_case = AIR_COM_PACKET__PAYLOAD_VARIANT_NODE_INFO;
        packet.node_info = &node_info;

        node_info.callsign = (char*)CALLSIGN;
        uint8_t mac[6];
        esp_efuse_mac_get_default(mac);
        char uid[32];
        sprintf(uid, "ESP32-%02x%02x%02x", mac[3], mac[4], mac[5]);
        node_info.node_id = uid;

        // 2. Serialize the packet to a byte buffer.
        size_t packed_size = air_com_packet__get_packed_size(&packet);
        uint8_t *buffer = (uint8_t *)malloc(packed_size);
        if (buffer == NULL) {
            ESP_LOGE(TAG, "Failed to allocate buffer for protobuf packet");
            log_message(LOG_LEVEL_ERROR, "Failed to allocate buffer for protobuf packet");
            vTaskDelay(pdMS_TO_TICKS(1000)); // Wait before retry
            continue;
        }
        air_com_packet__pack(&packet, buffer);

        // 3. Broadcast the serialized packet using network utilities
        if (!broadcast_udp_packet(buffer, packed_size, MESH_DISCOVERY_PORT)) {
            ESP_LOGE(TAG, "Failed to broadcast discovery packet");
            log_message(LOG_LEVEL_ERROR, "Failed to broadcast discovery packet");
        }
        free(buffer);

        // 4. Update our contact list from mesh nodes
        auto nodes = meshManager.getMeshNodes();

        // Update the global contact list with improved mutex handling
        if (xSemaphoreTake(g_contact_list_mutex, MUTEX_TIMEOUT_DEFAULT) == pdTRUE) {
            g_contact_list.clear();
            for(const auto& node : nodes) {
                // In a real implementation, we'd get the callsign from the parsed JSON
                g_contact_list.push_back({ .callsign = "CONTACT-" + std::to_string(g_contact_list.size() + 1), .ipAddress = node.ipv6Address });
            }
            xSemaphoreGive(g_contact_list_mutex);
        } else {
            ESP_LOGE(TAG, "Failed to get contact list mutex within timeout");
            log_message(LOG_LEVEL_WARN, "Failed to get contact list mutex within timeout");
        }

        // Send a simple UI update notification for the contact count
        ui_update_t update = { .has_gps_lock = (bool)0xFF, .contact_count = (uint8_t)nodes.size() };
        xQueueSend(ui_update_queue, &update, (TickType_t)0);

        // Check for outgoing text messages to send
        outgoing_message_t out_msg;
        if (xQueueReceive(outgoing_message_queue, (TickType_t)0) == pdPASS) {
            ESP_LOGI(TAG, "Dequeued a message to send to %s", out_msg.target_ip);

            // Use network utilities to send TCP message
            if (!send_tcp_message_default(out_msg.target_ip, out_msg.encrypted_payload)) {
                ESP_LOGE(TAG, "Failed to send TCP message to %s", out_msg.target_ip);
                log_message(LOG_LEVEL_ERROR, "Failed to send TCP message");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Shorten delay to be more responsive to outgoing messages
    }
}

/**
 * @brief TCP server task for receiving messages
 *
 * This task listens for incoming TCP connections and processes received messages.
 */
void tcp_server_task(void *pvParameters) {
    ESP_LOGI(TAG, "TCP server task started");

    // Initialize network utilities if not already done
    if (network_utils_init() != true) {
        ESP_LOGE(TAG, "Failed to initialize network utilities");
        vTaskDelete(NULL);
        return;
    }

    char rx_buffer[1024];
    char addr_str[128];
    int addr_family = AF_INET;
    int ip_protocol = 0;
    struct sockaddr_in6 dest_addr;

    struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
    dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr_ip4->sin_family = AF_INET;
    dest_addr_ip4->sin_port = htons(TEXT_PORT);
    ip_protocol = IPPROTO_IP;

    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        log_message(LOG_LEVEL_ERROR, "Unable to create TCP server socket");
        vTaskDelete(NULL);
        return;
    }
    ESP_LOGI(TAG, "Socket created");

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        log_message(LOG_LEVEL_ERROR, "TCP server socket bind failed");
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", TEXT_PORT);

    err = listen(listen_sock, 1);
    if (err != 0) {
        ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        log_message(LOG_LEVEL_ERROR, "TCP server listen failed");
        goto CLEAN_UP;
    }

    for(;;) {
        ESP_LOGI(TAG, "Socket listening for text messages...");

        struct sockaddr_in6 source_addr; // Large enough for both IPv4 or IPv6
        uint addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            log_message(LOG_LEVEL_ERROR, "TCP server accept failed");
            break;
        }

        // Convert client address to a readable string
        if (source_addr.sin6_family == PF_INET) {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
        } else if (source_addr.sin6_family == PF_INET6) {
            inet6_ntoa_r(source_addr.sin6_addr, addr_str, sizeof(addr_str) - 1);
        }
        ESP_LOGI(TAG, "Socket accepted ip address: %s", addr_str);

        int len;
        std::vector<uint8_t> received_data;
        do {
            len = recv(sock, rx_buffer, sizeof(rx_buffer), 0);
            if (len > 0) {
                received_data.insert(received_data.end(), rx_buffer, rx_buffer + len);
            }
        } while (len > 0);

        if (len < 0) {
            ESP_LOGE(TAG, "recv failed: errno %d", errno);
            log_message(LOG_LEVEL_ERROR, "TCP receive failed");
        } else if (received_data.size() > 0) {
            ESP_LOGI(TAG, "Received %d bytes", received_data.size());
            // Pass the raw buffer to the centralized handler
            handle_incoming_packet(received_data.data(), received_data.size());
        }

        // Ensure socket is always closed
        shutdown(sock, 0);
        close(sock);
        sock = -1; // Mark as closed
    }

CLEAN_UP:
    if (listen_sock >= 0) {
        close(listen_sock);
        listen_sock = -1; // Mark as closed
    }
    vTaskDelete(NULL);
}