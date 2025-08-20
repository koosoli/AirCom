#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "include/config.h"
#include "include/gps_task.h"
#include "include/atak_task.h"
#include "include/ui_task.h"
#include "include/audio_task.h"
#include "include/shared_data.h"
#include "HaLowMeshManager.h"
#include "AirCom.pb-c.h"
#include "crypto.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>


// C++ entry point
extern "C" void app_main(void);

// Task Handles
static TaskHandle_t networkTaskHandle = NULL;
static TaskHandle_t tcpServerTaskHandle = NULL;
static TaskHandle_t atakTaskHandle = NULL;
static TaskHandle_t uiTaskHandle = NULL;
static TaskHandle_t audioTaskHandle = NULL;
static TaskHandle_t gpsTaskHandle = NULL;

void networkTask(void *pvParameters) {
    ESP_LOGI(TAG, "networkTask started");

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
        air_com_packet__pack(&packet, buffer);

        // 3. Broadcast the serialized packet.
        meshManager.sendUdpMulticast(buffer, packed_size, MESH_DISCOVERY_PORT);
        free(buffer);

        // 4. In a real implementation, we would listen for discovery UDP packets from others,
        // unpack them, and update our contact list.
        // For this stub, we will continue to use the dummy list from the HaLow manager.
        auto nodes = meshManager.getMeshNodes();

        // Update the global contact list
        if (xSemaphoreTake(g_contact_list_mutex, (TickType_t)10) == pdTRUE) {
            g_contact_list.clear();
            for(const auto& node : nodes) {
                // In a real implementation, we'd get the callsign from the parsed JSON
                g_contact_list.push_back({ .callsign = "CONTACT-" + std::to_string(g_contact_list.size() + 1), .ipAddress = node.ipv6Address });
            }
            xSemaphoreGive(g_contact_list_mutex);
        } else {
            ESP_LOGE(TAG, "Failed to get contact list mutex");
        }

        // Send a simple UI update notification for the contact count
        ui_update_t update = { .has_gps_lock = (bool)0xFF, .contact_count = (uint8_t)nodes.size() };
        xQueueSend(ui_update_queue, &update, (TickType_t)0);

        // Check for outgoing text messages to send
        outgoing_message_t out_msg;
        if (xQueueReceive(outgoing_message_queue, &out_msg, (TickType_t)0) == pdPASS) {
            ESP_LOGI(TAG, "Dequeued a message to send to %s", out_msg.target_ip);
            send_tcp_message(out_msg.target_ip, out_msg.encrypted_payload);
        }


        vTaskDelay(pdMS_TO_TICKS(100)); // Shorten delay to be more responsive to outgoing messages
    }
}

static void send_tcp_message(const char* host_ip, const std::vector<uint8_t>& payload) {
    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(host_ip);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(TEXT_PORT);

    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return;
    }
    ESP_LOGI(TAG, "Socket created, connecting to %s:%d", host_ip, TEXT_PORT);

    int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
        close(sock);
        return;
    }
    ESP_LOGI(TAG, "Successfully connected");

    err = send(sock, payload.data(), payload.size(), 0);
    if (err < 0) {
        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
    }

    shutdown(sock, 0);
    close(sock);
}


void tcp_server_task(void *pvParameters) {
    ESP_LOGI(TAG, "TCP server task started");
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
        vTaskDelete(NULL);
        return;
    }
    ESP_LOGI(TAG, "Socket created");

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", TEXT_PORT);

    err = listen(listen_sock, 1);
    if (err != 0) {
        ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        goto CLEAN_UP;
    }

    for(;;) {
        ESP_LOGI(TAG, "Socket listening for text messages...");

        struct sockaddr_in6 source_addr; // Large enough for both IPv4 or IPv6
        uint addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
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
        } else {
            ESP_LOGI(TAG, "Received %d bytes", received_data.size());

            // Decrypt and unpack the message
            std::string decrypted_payload = decrypt_message(received_data);
            if (!decrypted_payload.empty()) {
                AirComPacket *packet = air_com_packet__unpack(NULL, decrypted_payload.size(), (const uint8_t*)decrypted_payload.c_str());
                if (packet) {
                    if (packet->payload_variant_case == AIR_COM_PACKET__PAYLOAD_VARIANT_TEXT_MESSAGE) {
                        ESP_LOGI(TAG, "Received Text Message: '%s'", packet->text_message->text);
                        incoming_message_t received_msg;
                        received_msg.sender_callsign = packet->from_node;
                        received_msg.message_text = packet->text_message->text;
                        xQueueSend(incoming_message_queue, &received_msg, (TickType_t)0);
                    }
                    air_com_packet__free_unpacked(packet, NULL);
                } else {
                    ESP_LOGE(TAG, "Failed to unpack protobuf packet");
                }
            } else {
                ESP_LOGE(TAG, "Failed to decrypt message or empty payload");
            }
        }

        shutdown(sock, 0);
        close(sock);
    }

CLEAN_UP:
    close(listen_sock);
    vTaskDelete(NULL);
}


void app_main(void)
{
    ESP_LOGI(TAG, "Welcome to Project AirCom (ESP-IDF)!");

    // Initialize shared data structures (like queues)
    shared_data_init();

    // Create FreeRTOS tasks
    ESP_LOGI(TAG, "Creating tasks...");

    // Core 0: Network and data processing
    xTaskCreatePinnedToCore(networkTask, "Network", STACK_SIZE_DEFAULT, NULL, 5, &networkTaskHandle, 0);
    xTaskCreatePinnedToCore(tcp_server_task, "TCPServer", STACK_SIZE_DEFAULT, NULL, 5, &tcpServerTaskHandle, 0);
    xTaskCreatePinnedToCore(atakTask, "ATAK", STACK_SIZE_DEFAULT, NULL, 5, &atakTaskHandle, 0);

    // Core 1: Real-time UI and peripherals
    xTaskCreatePinnedToCore(uiTask, "UI", STACK_SIZE_DEFAULT, NULL, 5, &uiTaskHandle, 1);
    xTaskCreatePinnedToCore(audioTask, "Audio", STACK_SIZE_DEFAULT, NULL, 20, &audioTaskHandle, 1); // Higher priority for audio
    xTaskCreatePinnedToCore(gpsTask, "GPS", STACK_SIZE_DEFAULT, NULL, 5, &gpsTaskHandle, 1);

    ESP_LOGI(TAG, "All tasks created.");
}
