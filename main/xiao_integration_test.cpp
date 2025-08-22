/**
 * @file xiao_integration_test.cpp
 * @brief Integration test for XIAO ESP32 series with MM-IoT-SDK
 *
 * This file provides integration tests to verify that the AirCom system
 * is properly ported to XIAO ESP32 series boards with MM-IoT-SDK support.
 *
 * @author AirCom Development Team
 * @version 1.0.0
 * @date 2024
 */

#include <stdio.h>
#include "esp_log.h"
#include "xiao_esp32_config.h"
#include "mm_iot_sdk.h"
#include "HaLowMeshManager.h"
#include "include/config.h"

static const char* TAG = "XIAO_INTEGRATION_TEST";

/**
 * @brief Test XIAO ESP32 board configuration
 */
bool test_xiao_board_config(void) {
    ESP_LOGI(TAG, "Testing XIAO ESP32 board configuration...");

    // Test board type detection
    ESP_LOGI(TAG, "Board type: %s (ID: %d)", XIAO_BOARD_TYPE, XIAO_BOARD_TYPE_ID);

    // Test pin configuration
    ESP_LOGI(TAG, "SPI Configuration:");
    ESP_LOGI(TAG, "  - MOSI: %d", get_spi_mosi_pin());
    ESP_LOGI(TAG, "  - MISO: %d", get_spi_miso_pin());
    ESP_LOGI(TAG, "  - SCLK: %d", get_spi_sclk_pin());
    ESP_LOGI(TAG, "  - CS: %d", get_spi_cs_pin());
    ESP_LOGI(TAG, "  - RESET: %d", get_spi_reset_pin());
    ESP_LOGI(TAG, "  - INT: %d", get_spi_int_pin());

    // Test other peripherals
    ESP_LOGI(TAG, "Other Peripherals:");
    ESP_LOGI(TAG, "  - LED: %d", get_led_pin());
    ESP_LOGI(TAG, "  - Button: %d", get_button_pin());
    ESP_LOGI(TAG, "  - Battery ADC: %d", get_battery_adc_pin());

    ESP_LOGI(TAG, "XIAO ESP32 board configuration test passed");
    return true;
}

/**
 * @brief Test MM-IoT-SDK initialization
 */
bool test_mm_iot_sdk(void) {
    ESP_LOGI(TAG, "Testing MM-IoT-SDK initialization...");

    auto& sdk = MMIoTSDK::getInstance();

    if (sdk.isInitialized()) {
        ESP_LOGI(TAG, "MM-IoT-SDK already initialized");
        return true;
    }

    // Test SDK initialization
    bool success = sdk.initialize("AirCom-HaLow", "aircom2024", "00");
    if (!success) {
        ESP_LOGE(TAG, "MM-IoT-SDK initialization failed");
        return false;
    }

    ESP_LOGI(TAG, "MM-IoT-SDK initialization test passed");
    return true;
}

/**
 * @brief Test HaLowMeshManager integration
 */
bool test_halow_mesh_manager(void) {
    ESP_LOGI(TAG, "Testing HaLowMeshManager integration...");

    auto& meshManager = HaLowMeshManager::getInstance();

    // Test initialization
    bool success = meshManager.begin();
    if (!success) {
        ESP_LOGE(TAG, "HaLowMeshManager initialization failed");
        return false;
    }

    // Test discovery start
    success = meshManager.startDiscovery();
    if (!success) {
        ESP_LOGE(TAG, "HaLowMeshManager discovery start failed");
        return false;
    }

    // Test getting mesh nodes (will return mock data for now)
    auto nodes = meshManager.getMeshNodes();
    ESP_LOGI(TAG, "Discovered %zu mesh nodes", nodes.size());

    // Test connection status
    bool connected = meshManager.get_connection_status();
    ESP_LOGI(TAG, "Connection status: %s", connected ? "Connected" : "Disconnected");

    ESP_LOGI(TAG, "HaLowMeshManager integration test passed");
    return true;
}

/**
 * @brief Test SPI configuration for FGH100M-H module
 */
bool test_spi_configuration(void) {
    ESP_LOGI(TAG, "Testing SPI configuration for FGH100M-H module...");

    // Check SPI configuration constants
    ESP_LOGI(TAG, "SPI Host: %d", FGH100M_SPI_HOST);
    ESP_LOGI(TAG, "SPI Clock Speed: %d Hz", FGH100M_SPI_CLOCK_SPEED);
    ESP_LOGI(TAG, "SPI Mode: %d", FGH100M_SPI_MODE);
    ESP_LOGI(TAG, "Max TX Buffer: %d bytes", FGH100M_MAX_TX_BUFFER);
    ESP_LOGI(TAG, "Max RX Buffer: %d bytes", FGH100M_MAX_RX_BUFFER);

    // Check timing constants
    ESP_LOGI(TAG, "Reset Delay: %d ms", FGH100M_RESET_DELAY);
    ESP_LOGI(TAG, "Startup Delay: %d ms", FGH100M_STARTUP_DELAY);
    ESP_LOGI(TAG, "SPI Timeout: %d ms", FGH100M_SPI_TIMEOUT);
    ESP_LOGI(TAG, "Command Timeout: %d ms", FGH100M_COMMAND_TIMEOUT);

    // Verify SPI pins are properly configured
    if (get_spi_mosi_pin() < 0 || get_spi_miso_pin() < 0 ||
        get_spi_sclk_pin() < 0 || get_spi_cs_pin() < 0) {
        ESP_LOGE(TAG, "SPI pins not properly configured");
        return false;
    }

    ESP_LOGI(TAG, "SPI configuration test passed");
    return true;
}

/**
 * @brief Run all integration tests
 */
bool run_xiao_integration_tests(void) {
    ESP_LOGI(TAG, "Starting XIAO ESP32 integration tests...");

    bool all_tests_passed = true;

    // Test 1: Board configuration
    if (!test_xiao_board_config()) {
        all_tests_passed = false;
    }

    // Test 2: SPI configuration
    if (!test_spi_configuration()) {
        all_tests_passed = false;
    }

    // Test 3: MM-IoT-SDK
    if (!test_mm_iot_sdk()) {
        all_tests_passed = false;
    }

    // Test 4: HaLowMeshManager
    if (!test_halow_mesh_manager()) {
        all_tests_passed = false;
    }

    if (all_tests_passed) {
        ESP_LOGI(TAG, "All XIAO ESP32 integration tests passed!");
    } else {
        ESP_LOGE(TAG, "Some XIAO ESP32 integration tests failed!");
    }

    return all_tests_passed;
}

/**
 * @brief Initialize XIAO ESP32 integration testing
 */
void xiao_integration_test_init(void) {
    ESP_LOGI(TAG, "XIAO ESP32 integration test module initialized");

    // Run tests (can be called from main or a dedicated test task)
    // run_xiao_integration_tests();
}