#include "include/atak_task.h"
#include "include/config.h"
#include "include/gps_task.h"
#include "HaLowMeshManager.h"
#include "AirCom.pb-c.h"
#include "esp_log.h"
#include "esp_system.h" // For MAC address
#include <string>
#include <time.h>

// Helper function to get current time in ISO 8601 format
// Note: This requires the system time to be set, ideally from GPS or NTP.
static std::string getISO8601Time(time_t timestamp) {
    char buf[sizeof "2011-10-08T07:07:09Z"];
    strftime(buf, sizeof buf, "%Y-%m-%dT%H:%M:%SZ", gmtime(&timestamp));
    return std::string(buf);
}

// Helper to construct the CoT XML message
static std::string generateCoT(const GPSData& gpsData) {
    char uid[32];
    uint8_t mac[6];
    esp_efuse_mac_get_default(mac);
    sprintf(uid, "ESP32-%02x%02x%02x", mac[3], mac[4], mac[5]);

    time_t now;
    time(&now); // Get current time
    time_t stale = now + 60; // Stale time 60 seconds from now

    // Using std::string for easier concatenation
    std::string cot = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    cot += "<event version=\"2.0\" uid=\"";
    cot += uid;
    cot += "\" type=\"a-f-G-E-V-C\" time=\"";
    cot += getISO8601Time(now);
    cot += "\" start=\"";
    cot += getISO8601Time(now);
    cot += "\" stale=\"";
    cot += getISO8601Time(stale);
    cot += "\" how=\"h-e\">";
    cot += "<point lat=\"" + std::to_string(gpsData.latitude) +
           "\" lon=\"" + std::to_string(gpsData.longitude) +
           "\" hae=\"9999999.0\" ce=\"5\" le=\"9999999.0\"/>";
    cot += "<detail>";
    cot += "<contact callsign=\"" CALLSIGN "\"/>";
    cot += "<uid Droid=\"" CALLSIGN "\"/>";
    cot += "<__group name=\"Cyan\" role=\"Team Member\"/>";
    cot += "</detail>";
    cot += "</event>";

    return cot;
}

void atakTask(void *pvParameters) {
    ESP_LOGI(TAG, "atakTask started");

    HaLowMeshManager& meshManager = HaLowMeshManager::getInstance();

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(10000)); // Run every 10 seconds

        GPSData data = gps_get_data();
        if (data.isValid) {
            std::string cot_xml = generateCoT(data);

            // 1. Create the protobuf packet
            AirComPacket packet = AIR_COM_PACKET__INIT;
            packet.payload_variant_case = AIR_COM_PACKET__PAYLOAD_VARIANT_COT_MESSAGE;
            packet.cot_message = (char*)cot_xml.c_str();

            // 2. Serialize the packet
            size_t packed_size = air_com_packet__get_packed_size(&packet);
            uint8_t *buffer = (uint8_t *)malloc(packed_size);
            air_com_packet__pack(&packet, buffer);

            // 3. Broadcast the serialized packet.
            ESP_LOGI(TAG, "Broadcasting CoT protobuf message...");
            meshManager.sendUdpMulticast(buffer, packed_size, ATAK_PORT);
            free(buffer);

        } else {
            ESP_LOGW(TAG, "ATAK task: No valid GPS lock, skipping broadcast.");
        }
    }
}
