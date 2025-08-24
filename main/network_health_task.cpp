#include "include/network_health_task.h"
#include "include/config.h"
#include "include/network_utils.h"
#include "include/error_handling.h"
#include "HaLowMeshManager.h"
#include "AirCom.pb-c.h"
#include "esp_log.h"
#include "esp_system.h" // For esp_efuse_mac_get_default

static const char* TAG = "NET_HEALTH_TASK";

#define HEALTH_BROADCAST_INTERVAL_MS 30000 // Broadcast every 30 seconds

void network_health_task(void *pvParameters) {
    ESP_LOGI(TAG, "Network Health Task started");

    // Allow time for the main network task to initialize the mesh
    vTaskDelay(pdMS_TO_TICKS(10000));

    for (;;) {
        HaLowMeshManager& meshManager = HaLowMeshManager::getInstance();
        if (!meshManager.get_connection_status()) {
            ESP_LOGW(TAG, "HaLow mesh is not connected. Skipping health broadcast.");
            vTaskDelay(pdMS_TO_TICKS(HEALTH_BROADCAST_INTERVAL_MS));
            continue;
        }

        // 1. Create a NetworkHealth packet.
        AirComPacket packet = AIR_COM_PACKET__INIT;
        NetworkHealth health_info = NETWORK_HEALTH__INIT;

        packet.payload_variant_case = AIR_COM_PACKET__PAYLOAD_VARIANT_NETWORK_HEALTH;
        packet.network_health = &health_info;

        // Set the sender's node ID
        uint8_t mac[6];
        esp_efuse_mac_get_default(mac);
        char uid[32];
        sprintf(uid, "ESP32-%02x%02x%02x", mac[3], mac[4], mac[5]);
        packet.from_node = uid;

        // In a real implementation, we would iterate through connected peers
        // and get the specific RSSI for each link. For this skeleton, we'll
        // just broadcast a placeholder value.
        health_info.rssi = meshManager.get_rssi(); // Assuming a general RSSI for now

        // 2. Serialize the packet to a byte buffer.
        size_t packed_size = air_com_packet__get_packed_size(&packet);
        uint8_t *buffer = (uint8_t *)malloc(packed_size);
        if (buffer == NULL) {
            ESP_LOGE(TAG, "Failed to allocate buffer for health packet");
            log_message(LOG_LEVEL_ERROR, "Failed to allocate buffer for health packet");
            vTaskDelay(pdMS_TO_TICKS(HEALTH_BROADCAST_INTERVAL_MS));
            continue;
        }
        air_com_packet__pack(&packet, buffer);

        // 3. Broadcast the serialized packet using network utilities.
        // We use the same discovery port for simplicity.
        ESP_LOGI(TAG, "Broadcasting network health packet (RSSI: %d)", health_info.rssi);
        if (!broadcast_udp_packet(buffer, packed_size, MESH_DISCOVERY_PORT)) {
            ESP_LOGE(TAG, "Failed to broadcast health packet");
            log_message(LOG_LEVEL_ERROR, "Failed to broadcast health packet");
        }
        free(buffer);

        // 4. Wait for the next interval.
        vTaskDelay(pdMS_TO_TICKS(HEALTH_BROADCAST_INTERVAL_MS));
    }
}
