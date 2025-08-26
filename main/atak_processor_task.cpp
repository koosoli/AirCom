/**
 * @file atak_processor_task.cpp
 * @brief ATAK processor task implementation for AirCom system
 *
 * This file contains the ATAK message processing functionality extracted from main.cpp
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
#include "include/error_handling.h"
#include "AirCom.pb-c.h"

#include <lwip/err.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/netdb.h>

static const char* ATAK_PROC_TAG = "ATAK_PROC";

// ============================================================================
// ATAK PROCESSOR TASK IMPLEMENTATION
// ============================================================================

/**
 * @brief Parse a value from a CoT XML string
 *
 * @param cot CoT XML string to parse
 * @param key XML attribute key to find
 * @return Extracted value string, empty string if not found
 */
static std::string parse_cot_value(const std::string& cot, const char* key) {
    size_t key_pos = cot.find(key);
    if (key_pos == std::string::npos) return "";
    key_pos += strlen(key); // Move to the start of the value
    size_t end_quote_pos = cot.find('"', key_pos);
    if (end_quote_pos == std::string::npos) return "";
    return cot.substr(key_pos, end_quote_pos - key_pos);
}

/**
 * @brief ATAK processor task
 *
 * This task receives CoT messages from other clients on the network
 * and updates the shared list of teammate locations.
 *
 * @param pvParameters FreeRTOS task parameters
 */
void atak_processor_task(void *pvParameters) {
    LOG_INFO(ATAK_PROC_TAG, "ATAK Processor task started");

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) {
        LOG_ERROR(ATAK_PROC_TAG, ERROR_SOCKET_CREATE, "ATAK RX: Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return; // Ensure task terminates
    }

    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(ATAK_PORT);
    int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err < 0) {
        LOG_ERROR(ATAK_PROC_TAG, ERROR_SOCKET_BIND, "ATAK RX: Socket unable to bind: errno %d", errno);
        close(sock); // Clean up socket before exiting
        vTaskDelete(NULL);
        return; // Ensure task terminates
    }

    for(;;) {
        struct sockaddr_in source_addr;
        socklen_t socklen = sizeof(source_addr);
        uint8_t rx_buffer[1500];
        int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

        if (len > 0) {
            AirComPacket *packet = air_com_packet__unpack(NULL, len, rx_buffer);
            if (packet != NULL) {
                if (packet->payload_variant_case == AIR_COM_PACKET__PAYLOAD_VARIANT_COT_MESSAGE) {
                    LOG_INFO(ATAK_PROC_TAG, "Received CoT message");

                    TeammateInfo new_info;
                    std::string cot_xml = packet->cot_message;

                    new_info.callsign = parse_cot_value(cot_xml, "callsign=\"");
                    new_info.lat = std::stod(parse_cot_value(cot_xml, "lat=\""));
                    new_info.lon = std::stod(parse_cot_value(cot_xml, "lon=\""));
                    new_info.last_update_time = pdTICKS_TO_MS(xTaskGetTickCount());

                    if (xSemaphoreTake(g_teammate_locations_mutex, (TickType_t)10) == pdTRUE) {
                        bool found = false;
                        for (auto& teammate : g_teammate_locations) {
                            if (teammate.callsign == new_info.callsign) {
                                teammate = new_info;
                                found = true;
                                break;
                            }
                        }
                        if (!found) {
                            g_teammate_locations.push_back(new_info);
                        }
                        xSemaphoreGive(g_teammate_locations_mutex);
                    } else {
                        LOG_WARNING(ATAK_PROC_TAG, "Failed to acquire teammate locations mutex");
                    }
                }
                air_com_packet__free_unpacked(packet, NULL);
            } else {
                LOG_ERROR(ATAK_PROC_TAG, ERROR_INVALID_PARAMETER, "Failed to unpack CoT packet - possible memory leak prevented");
            }
        } else if (len < 0) {
            LOG_ERROR(ATAK_PROC_TAG, ERROR_SOCKET_RECEIVE, "ATAK recvfrom failed: errno %d", errno);
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Prevent busy-looping
    }

    close(sock);
    vTaskDelete(NULL);
}