#include "include/network_health_task.h"
#include "include/shared_data.h"
#include "include/network_utils.h" // For MESH_DISCOVERY_PORT
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "AirCom.pb-c.h"

static const char* TAG = "NET_HEALTH";
static const int HEALTH_CHECK_INTERVAL_S = 30; // Interval in seconds to send health checks

void network_health_task(void *pvParameters) {
    ESP_LOGI(TAG, "Network Health Task started");

    while (1) {
        // Delay for the specified interval
        vTaskDelay(pdMS_TO_TICKS(HEALTH_CHECK_INTERVAL_S * 1000));

        ESP_LOGI(TAG, "Running periodic health check...");

        // 1. Lock the contact list to get the list of peers
        if (xSemaphoreTake(g_contact_list_mutex, portMAX_DELAY) == pdTRUE) {

            // 2. Iterate through all known peers
            for (const auto& pair : g_contact_list) {
                const MeshNodeInfo& node = pair.second;
                ESP_LOGD(TAG, "Queueing health check for node: %s", node.node_id.c_str());

                // Create the HealthCheckRequest
                HealthCheckRequest health_req = HEALTH_CHECK_REQUEST__INIT;
                health_req.sent_timestamp_ms = esp_log_timestamp();

                // Create the main AirComPacket
                AirComPacket packet = AIR_COM_PACKET__INIT;
                packet.payload_variant_case = AIR_COM_PACKET__PAYLOAD_VARIANT_HEALTH_REQUEST;
                packet.health_request = &health_req;
                // TODO: Set from_node ID
                // packet.from_node = get_my_node_id();
                packet.to_node = (char*)node.node_id.c_str();

                // Serialize the packet
                size_t packed_size = air_com_packet__get_packed_size(&packet);
                std::vector<uint8_t> buffer(packed_size);
                air_com_packet__pack(&packet, buffer.data());

                // Queue the packet for sending.
                // NOTE: The current network_task sends via TCP. This is not ideal for health
                // checks, which should be UDP. This highlights a need to refactor network_task
                // to support sending different transport protocols based on packet type.
                outgoing_message_t out_msg;
                strncpy(out_msg.target_ip, node.ipAddress.c_str(), sizeof(out_msg.target_ip) - 1);
                out_msg.encrypted_payload = buffer; // This is not encrypted, but uses the same pipe for now.

                if (send_outgoing_message(&out_msg) != pdPASS) {
                    ESP_LOGW(TAG, "Failed to queue health check packet for %s", node.node_id.c_str());
                }
            }

            // 5. Release the mutex
            xSemaphoreGive(g_contact_list_mutex);
        }

        // 6. Prune stale entries from the stats map
        //    - Lock the stats map mutex
        //    - Iterate through the map and remove entries that haven't been updated in a while
        //    - Release the mutex
        //    - Placeholder for pruning logic
    }
}
