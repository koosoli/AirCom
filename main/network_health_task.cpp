#include "include/network_health_task.h"
#include "include/shared_data.h"
#include "include/network_task.h" // To get access to the packet sending queue
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

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
                ESP_LOGD(TAG, "Sending health check to node: %s", node.node_id.c_str());

                // 3. Create a HealthCheckRequest packet
                //    - This will involve creating the protobuf message
                //    - Then wrapping it in an AirComPacket
                //    - The actual implementation will be done in a subsequent step

                // 4. Send the packet to the network task's outgoing queue
                //    - Placeholder for sending logic
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
