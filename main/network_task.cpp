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
#include "HaLowManager/include/HaLowMeshManager.h"
#include "logging_system.h"
#include "lwip/sockets.h"
#include "esp_efuse.h"
#include "esp_mac.h"


// Define mutex timeout constants locally (should be in shared_data.h)
#define MUTEX_TIMEOUT_DEFAULT pdMS_TO_TICKS(500)

#include "AirCom.pb-c.h"

static const char* NETWORK_TASK_TAG = "NETWORK_TASK";

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
    ESP_LOGI(NETWORK_TASK_TAG, "networkTask started");

    // Initialize network utilities
    if (network_utils_init() != true) {
        ESP_LOGE(NETWORK_TASK_TAG, "Failed to initialize network utilities");
        vTaskDelete(NULL);
        return;
    }

    // Initialize the HaLow Mesh Manager
    HaLowMeshManager& meshManager = HaLowMeshManager::getInstance();
    meshManager.begin();

    // Main task loop
    for (;;) {
        ESP_LOGI(NETWORK_TASK_TAG, "Broadcasting discovery packet...");

        // 1. Create a NodeInfo packet to broadcast our presence.
        AirComPacket packet = AIR_COM_PACKET__INIT;
        NodeInfo node_info = NODE_INFO__INIT;

        packet.payload_variant_case = AIR_COM_PACKET__PAYLOAD_VARIANT_NODE_INFO;
        packet.node_info = &node_info;

        node_info.callsign = (char*)CALLSIGN;
        uint8_t mac[6];
        esp_read_mac(mac, ESP_MAC_WIFI_STA);
        char uid[32];
        sprintf(uid, "ESP32-%02x%02x%02x", mac[3], mac[4], mac[5]);
        node_info.node_id = uid;

        // 2. Serialize the packet to a byte buffer.
        size_t packed_size = air_com_packet__get_packed_size(&packet);
        uint8_t *buffer = (uint8_t *)malloc(packed_size);
        if (buffer == NULL) {
            LOG_NETWORK_ERROR(ERROR_MEMORY_ALLOCATION, "Failed to allocate buffer for protobuf packet");
            vTaskDelay(pdMS_TO_TICKS(1000)); // Wait before retry
            continue;
        }
        air_com_packet__pack(&packet, buffer);

        // 3. Broadcast the serialized packet using network utilities
        if (!broadcast_udp_packet(buffer, packed_size, MESH_DISCOVERY_PORT)) {
            LOG_NETWORK_ERROR(ERROR_SOCKET_SEND, "Failed to broadcast discovery packet");
        }
        free(buffer);

        // 4. Listen for incoming UDP packets (for discovery and health)
        uint8_t rx_buffer[512];
        char source_ip[40];
        int len = receive_udp_packet(rx_buffer, sizeof(rx_buffer), source_ip, sizeof(source_ip));
        if (len > 0) {
            AirComPacket *received_packet = air_com_packet__unpack(NULL, len, rx_buffer);
            if (received_packet) {
                if (received_packet->payload_variant_case == AIR_COM_PACKET__PAYLOAD_VARIANT_NODE_INFO) {
                    // This is a discovery packet from another node.
                    // In a real implementation, we would add this to our peer list.
                    ESP_LOGI(NETWORK_TASK_TAG, "Received NodeInfo from %s (Callsign: %s)", received_packet->from_node, received_packet->node_info->callsign);
                } else if (received_packet->payload_variant_case == AIR_COM_PACKET__PAYLOAD_VARIANT_NETWORK_HEALTH) {
                    // This is a health packet.
                    ESP_LOGI(NETWORK_TASK_TAG, "Received NetworkHealth from %s (RSSI: %d)", received_packet->from_node, received_packet->network_health->rssi);
                    // In a real implementation, we would update a map of peer link statistics.
                }
                air_com_packet__free_unpacked(received_packet, NULL);
            }
        }


        // 5. Update our contact list from mesh nodes
        auto nodes = meshManager.getMeshNodes();

        // Update the global contact list with improved mutex handling
        if (xSemaphoreTake(g_contact_list_mutex, MUTEX_TIMEOUT_DEFAULT) == pdTRUE) {
            g_contact_list.clear();
            for(const auto& node : nodes) {
                // In a real implementation, we'd get the callsign from the parsed JSON
                MeshNodeInfo newNode;
                newNode.callsign = "CONTACT-" + std::to_string(g_contact_list.size() + 1);
                newNode.ipAddress = node.ipAddress;
                g_contact_list.push_back(newNode);
            }
            xSemaphoreGive(g_contact_list_mutex);
        } else {
            LOG_NETWORK_WARNING("Failed to get contact list mutex within timeout");
        }

        // Send a simple UI update notification for the contact count
        ui_update_t update = { .has_gps_lock = (bool)0xFF, .contact_count = (uint8_t)nodes.size() };
        xQueueSend(ui_update_queue, &update, (TickType_t)0);

        // Check for outgoing text messages to send
        outgoing_message_t out_msg;
        if (xQueueReceive(outgoing_message_queue, &out_msg, (TickType_t)0) == pdPASS) {
            ESP_LOGI(NETWORK_TASK_TAG, "Dequeued a message to send to %s", out_msg.target_ip);

            // Use network utilities to send TCP message
            if (!send_tcp_message_default(out_msg.target_ip, out_msg.encrypted_payload)) {
                LOG_NETWORK_ERROR(ERROR_SOCKET_SEND, "Failed to send TCP message to %s", out_msg.target_ip);
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
    ESP_LOGI(NETWORK_TASK_TAG, "TCP server task started");

    // Initialize network utilities if not already done
    if (network_utils_init() != true) {
        ESP_LOGE(NETWORK_TASK_TAG, "Failed to initialize network utilities");
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
        LOG_NETWORK_ERROR(ERROR_SOCKET_CREATE, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }
    ESP_LOGI(NETWORK_TASK_TAG, "Socket created");

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        LOG_NETWORK_ERROR(ERROR_SOCKET_BIND, "Socket unable to bind: errno %d", errno);
        goto CLEAN_UP;
    }
    ESP_LOGI(NETWORK_TASK_TAG, "Socket bound, port %d", TEXT_PORT);

    err = listen(listen_sock, 1);
    if (err != 0) {
        LOG_NETWORK_ERROR(ERROR_SOCKET_BIND, "Error occurred during listen: errno %d", errno);
        goto CLEAN_UP;
    }

    for(;;) {
        ESP_LOGI(NETWORK_TASK_TAG, "Socket listening for text messages...");

        struct sockaddr_in6 source_addr; // Large enough for both IPv4 or IPv6
        socklen_t addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) {
            LOG_NETWORK_ERROR(ERROR_SOCKET_CONNECT, "Unable to accept connection: errno %d", errno);
            break;
        }

        // Convert client address to a readable string
        if (source_addr.sin6_family == PF_INET) {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
        } else if (source_addr.sin6_family == PF_INET6) {
            inet6_ntoa_r(source_addr.sin6_addr, addr_str, sizeof(addr_str) - 1);
        }
        ESP_LOGI(NETWORK_TASK_TAG, "Socket accepted ip address: %s", addr_str);

        int len;
        std::vector<uint8_t> received_data;
        do {
            len = recv(sock, rx_buffer, sizeof(rx_buffer), 0);
            if (len > 0) {
                received_data.insert(received_data.end(), rx_buffer, rx_buffer + len);
            }
        } while (len > 0);

        if (len < 0) {
            LOG_NETWORK_ERROR(ERROR_SOCKET_RECEIVE, "recv failed: errno %d", errno);
        } else {
            ESP_LOGI(NETWORK_TASK_TAG, "Received %d bytes", received_data.size());

            // Decrypt and unpack the message
            std::string decrypted_payload = decrypt_message(received_data);
            if (!decrypted_payload.empty()) {
                AirComPacket *packet = air_com_packet__unpack(NULL, decrypted_payload.size(), (const uint8_t*)decrypted_payload.c_str());
                if (packet) {
                    if (packet->payload_variant_case == AIR_COM_PACKET__PAYLOAD_VARIANT_TEXT_MESSAGE) {
                        ESP_LOGI(NETWORK_TASK_TAG, "Received Text Message: '%s'", packet->text_message->text);
                        incoming_message_t received_msg;
                        received_msg.sender_callsign = packet->from_node;
                        received_msg.message_text = packet->text_message->text;
                        xQueueSend(incoming_message_queue, &received_msg, (TickType_t)0);
                    }
                    air_com_packet__free_unpacked(packet, NULL);
                } else {
                    LOG_NETWORK_ERROR(ERROR_INVALID_PARAMETER, "Failed to unpack protobuf packet");
                }
            } else {
                LOG_NETWORK_ERROR(ERROR_CRYPTO_DECRYPT, "Failed to decrypt message or empty payload");
            }
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