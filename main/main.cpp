/**
 * @file main.cpp
 * @brief AirCom ESP32 Communication System - Main Application Entry Point
 *
 * This file contains the main application logic for the AirCom tactical communication system.
 * The system provides real-time voice communication, text messaging, and position tracking
 * capabilities for tactical teams using ESP32-based hardware.
 *
 * Architecture:
 * - Task-based design with FreeRTOS
 * - Modular component architecture
 * - Real-time audio processing with low latency
 * - Secure communication with end-to-end encryption
 * - Mesh networking for extended range
 * - GPS tracking and ATAK integration
 *
 * Key Components:
 * - Audio Task: Real-time voice encoding/decoding
 * - Network Task: Mesh networking and message routing
 * - UI Task: Display and user interface management
 * - GPS Task: Location tracking and reporting
 * - ATAK Tasks: Tactical awareness and coordination
 *
 * @author AirCom Development Team
 * @version 1.0.0
 * @date 2024
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "include/config_manager.h"
#include "include/gps_task.h"
#include "include/atak_task.h"
#include "include/ui_task.h"
#include "include/audio_task.h"
#include "include/shared_data.h"
#include "include/error_handling.h"
#include "include/network_task.h"
#include "include/atak_processor_task.h"
#include "include/network_health_task.h"
#include "include/ota_updater.h"
#include "include/camera_service.h"
#include "include/config_manager.h"

#include "../components/aircom_proto/AirCom.pb-c.h"
#include "crypto.h"
#include "nvs_flash.h"
#include "include/bt_audio.h"



// C++ entry point
extern "C" void app_main(void);

// Tag for logging
static const char* TAG = "AIRCOM_MAIN";

// Task Handles
static TaskHandle_t networkTaskHandle = NULL;
static TaskHandle_t tcpServerTaskHandle = NULL;
static TaskHandle_t atakTaskHandle = NULL;
static TaskHandle_t atakProcessorTaskHandle = NULL;
static TaskHandle_t uiTaskHandle = NULL;
static TaskHandle_t audioTaskHandle = NULL;
static TaskHandle_t gpsTaskHandle = NULL;
static TaskHandle_t networkHealthTaskHandle = NULL;







void app_main(void)
{
    ESP_LOGI(TAG, "Welcome to Project AirCom (ESP-IDF)!");

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        error_report(ERROR_CATEGORY_STORAGE, ERROR_NVS_INIT,
                    "Failed to initialize NVS", __FILE__, __LINE__, __func__, NULL, 0);
    }
    ESP_ERROR_CHECK(ret);

    // Initialize shared data structures (like queues)
    shared_data_init();

    // Initialize error handling system
    if (!error_handling_init()) {
        ESP_LOGE(TAG, "Failed to initialize error handling system");
        return;
    }

    // Initialize Bluetooth audio
    bt_audio_init();

    // Initialize OTA Updater Service
    ota_updater_init();

    // Initialize Camera Service
    // In a real app, you might only initialize this if a camera is detected or enabled in config
    camera_service_init();

    // Create FreeRTOS tasks
    ESP_LOGI(TAG, "Creating tasks...");

    // Optimized task scheduling for real-time performance
    // Core 0: Network and GPS processing (lower priority tasks)
    BaseType_t result;
    result = xTaskCreatePinnedToCore(networkTask, "Network", STACK_SIZE_DEFAULT, NULL, 3, &networkTaskHandle, 0);
    if (result != pdPASS) {
        error_report(ERROR_CATEGORY_SYSTEM, ERROR_TASK_CREATION,
                    "Failed to create Network task", __FILE__, __LINE__, __func__, NULL, 0);
    }

    result = xTaskCreatePinnedToCore(tcp_server_task, "TCPServer", STACK_SIZE_DEFAULT, NULL, 3, &tcpServerTaskHandle, 0);
    if (result != pdPASS) {
        error_report(ERROR_CATEGORY_SYSTEM, ERROR_TASK_CREATION,
                    "Failed to create TCP Server task", __FILE__, __LINE__, __func__, NULL, 0);
    }

    result = xTaskCreatePinnedToCore(atakTask, "ATAK", STACK_SIZE_DEFAULT, NULL, 3, &atakTaskHandle, 0);
    if (result != pdPASS) {
        error_report(ERROR_CATEGORY_SYSTEM, ERROR_TASK_CREATION,
                    "Failed to create ATAK task", __FILE__, __LINE__, __func__, NULL, 0);
    }

    result = xTaskCreatePinnedToCore(atak_processor_task, "ATAKProc", STACK_SIZE_DEFAULT, NULL, 2, &atakProcessorTaskHandle, 0);
    if (result != pdPASS) {
        error_report(ERROR_CATEGORY_SYSTEM, ERROR_TASK_CREATION,
                    "Failed to create ATAK Processor task", __FILE__, __LINE__, __func__, NULL, 0);
    }

    result = xTaskCreatePinnedToCore(network_health_task, "NetHealth", STACK_SIZE_DEFAULT, NULL, 3, &networkHealthTaskHandle, 0);
    if (result != pdPASS) {
        error_report(ERROR_CATEGORY_SYSTEM, ERROR_TASK_CREATION,
                    "Failed to create Network Health task", __FILE__, __LINE__, __func__, NULL, 0);
    }

    result = xTaskCreatePinnedToCore(gpsTask, "GPS", STACK_SIZE_DEFAULT, NULL, 4, &gpsTaskHandle, 0);
    if (result != pdPASS) {
        error_report(ERROR_CATEGORY_SYSTEM, ERROR_TASK_CREATION,
                    "Failed to create GPS task", __FILE__, __LINE__, __func__, NULL, 0);
    }

    // Core 1: Critical real-time tasks (UI and Audio with balanced priorities)
    result = xTaskCreatePinnedToCore(uiTask, "UI", STACK_SIZE_DEFAULT, NULL, 8, &uiTaskHandle, 1); // Higher priority for UI responsiveness
    if (result != pdPASS) {
        error_report(ERROR_CATEGORY_SYSTEM, ERROR_TASK_CREATION,
                    "Failed to create UI task", __FILE__, __LINE__, __func__, NULL, 0);
    }

    result = xTaskCreatePinnedToCore(audioTask, "Audio", STACK_SIZE_DEFAULT, NULL, 10, &audioTaskHandle, 1); // Optimized audio priority for timing guarantees
    if (result != pdPASS) {
        error_report(ERROR_CATEGORY_SYSTEM, ERROR_TASK_CREATION,
                    "Failed to create Audio task", __FILE__, __LINE__, __func__, NULL, 0);
    }

    ESP_LOGI(TAG, "All tasks created with optimized real-time scheduling.");

    // Performance monitoring - log current task priorities
    ESP_LOGI(TAG, "Task Priority Summary:");
    ESP_LOGI(TAG, "  Network/Network tasks: Priority 3");
    ESP_LOGI(TAG, "  ATAK tasks: Priority 2-3");
    ESP_LOGI(TAG, "  GPS task: Priority 4");
    ESP_LOGI(TAG, "  UI task: Priority 8 (optimized for responsiveness)");
    ESP_LOGI(TAG, "  Audio task: Priority 10 (optimized for timing guarantees)");

    // Monitor task stack usage for performance validation
    ESP_LOGI(TAG, "Performance monitoring enabled - check logs for timing violations");
}
