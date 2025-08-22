/**
 * @file config_manager.cpp
 * @brief Unified configuration management system implementation
 *
 * This file implements the centralized configuration management system
 * that consolidates all pin definitions, settings, and runtime configuration.
 *
 * @author AirCom Development Team
 * @version 2.0.0
 * @date 2024
 */

#include "config_manager.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <cstring>
#include <string>

static const char* TAG = "CONFIG_MGR";

// NVS namespace for configuration storage
#define CONFIG_NVS_NAMESPACE "aircom_config"

// Global configuration instance
static aircom_config_t g_current_config;
static bool g_config_initialized = false;

// Forward declarations for platform-specific functions
extern "C" {
    // ESP-IDF specific functions
    uint32_t esp_random(void);
    esp_err_t nvs_get_str(nvs_handle_t handle, const char* key, char* out_value, size_t* length);
    esp_err_t nvs_set_str(nvs_handle_t handle, const char* key, const char* value);
    esp_err_t nvs_get_i32(nvs_handle_t handle, const char* key, int32_t* out_value);
    esp_err_t nvs_set_i32(nvs_handle_t handle, const char* key, int32_t value);
    esp_err_t nvs_get_u8(nvs_handle_t handle, const char* key, uint8_t* out_value);
    esp_err_t nvs_set_u8(nvs_handle_t handle, const char* key, uint8_t value);
}

// ============================================================================
// PLATFORM DETECTION FUNCTIONS
// ============================================================================

hardware_platform_t config_manager_detect_hardware(void) {
    // This is a simplified implementation
    // In a real system, you would detect based on:
    // - GPIO strapping pins
    // - EFUSE values
    // - Device tree information
    // - Build-time defines

#ifdef CONFIG_XIAO_ESP32S3
    return HW_PLATFORM_XIAO_ESP32S3;
#elif CONFIG_XIAO_ESP32C3
    return HW_PLATFORM_XIAO_ESP32C3;
#elif CONFIG_XIAO_ESP32C6
    return HW_PLATFORM_XIAO_ESP32C6;
#elif CONFIG_HELTEC_HT_HC32
    return HW_PLATFORM_HELTEC_HT_HC32;
#elif CONFIG_HELTEC_HT_IT01
    return HW_PLATFORM_HELTEC_HT_IT01;
#elif CONFIG_HELTEC_GENERIC
    return HW_PLATFORM_HELTEC_GENERIC;
#else
    // Default fallback - try to detect based on chip information
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    if (chip_info.model == CHIP_ESP32S3) {
        return HW_PLATFORM_XIAO_ESP32S3;
    } else if (chip_info.model == CHIP_ESP32C3) {
        return HW_PLATFORM_XIAO_ESP32C3;
    } else if (chip_info.model == CHIP_ESP32C6) {
        return HW_PLATFORM_XIAO_ESP32C6;
    } else {
        return HW_PLATFORM_ESP32_GENERIC;
    }
#endif
}

const char* config_manager_get_platform_name(hardware_platform_t platform) {
    switch (platform) {
        case HW_PLATFORM_XIAO_ESP32S3:
            return "XIAO ESP32S3";
        case HW_PLATFORM_XIAO_ESP32C3:
            return "XIAO ESP32C3";
        case HW_PLATFORM_XIAO_ESP32C6:
            return "XIAO ESP32C6";
        case HW_PLATFORM_HELTEC_HT_HC32:
            return "Heltec HT-HC32";
        case HW_PLATFORM_HELTEC_HT_IT01:
            return "Heltec HT-IT01";
        case HW_PLATFORM_HELTEC_GENERIC:
            return "Heltec Generic";
        case HW_PLATFORM_ESP32_GENERIC:
            return "ESP32 Generic";
        default:
            return "Unknown";
    }
}

bool config_manager_is_platform_supported(hardware_platform_t platform) {
    switch (platform) {
        case HW_PLATFORM_XIAO_ESP32S3:
        case HW_PLATFORM_XIAO_ESP32C3:
        case HW_PLATFORM_XIAO_ESP32C6:
        case HW_PLATFORM_HELTEC_HT_HC32:
        case HW_PLATFORM_HELTEC_HT_IT01:
        case HW_PLATFORM_HELTEC_GENERIC:
        case HW_PLATFORM_ESP32_GENERIC:
            return true;
        default:
            return false;
    }
}

// ============================================================================
// DEFAULT CONFIGURATION FUNCTIONS
// ============================================================================

bool config_manager_get_defaults(hardware_platform_t platform, aircom_config_t* config) {
    if (!config) {
        return false;
    }

    // Clear the configuration structure
    memset(config, 0, sizeof(aircom_config_t));

    // Set platform
    config->platform = platform;

    // Set common defaults
    config->network.ssid = "AirCom-HaLow";
    config->network.password = "aircom2024";
    config->network.country_code = "00";
    config->network.channel = 6;
    config->network.bandwidth = 20;
    config->network.enable_mesh = true;
    config->network.max_connections = 10;
    config->network.heartbeat_interval = 30000; // 30 seconds
    config->network.discovery_timeout = 5000;   // 5 seconds
    config->network.enable_encryption = true;
    config->network.encryption_key = "default_key_change_in_production";

    config->audio.sample_rate = 16000;
    config->audio.bits_per_sample = 16;
    config->audio.channels = 1;
    config->audio.buffer_size = 1024;
    config->audio.queue_depth = 5;
    config->audio.codec_bitrate = 32000;
    config->audio.enable_compression = true;
    config->audio.enable_noise_reduction = false;
    config->audio.ptt_debounce_ms = 50;

    config->display.width = 128;
    config->display.height = 64;
    config->display.rotation = 0;
    config->display.enable_backlight = true;
    config->display.backlight_timeout_ms = 30000;
    config->display.brightness = 128;
    config->display.enable_touch = false;
    config->display.font_name = "default";

    config->gps.baud_rate = 9600;
    config->gps.update_interval_ms = 1000;
    config->gps.enable_nmea_output = false;
    config->gps.enable_debug_output = false;
    config->gps.fix_timeout_ms = 120000; // 2 minutes
    config->gps.hdop_threshold = 5.0f;
    config->gps.enable_assisted_gps = false;

    config->system.log_level = 3; // ESP_LOG_INFO
    config->system.enable_performance_monitoring = true;
    config->system.enable_memory_tracking = true;
    config->system.watchdog_timeout_ms = 30000;
    config->system.task_stack_size_default = 4096;
    config->system.max_concurrent_connections = 5;
    config->system.device_name = "AirCom-Device";
    config->system.device_id = "AC-" + std::to_string(esp_random() % 1000000);
    config->system.firmware_version = 0x020000; // 2.0.0

    // Platform-specific pin configurations
    switch (platform) {
        case HW_PLATFORM_XIAO_ESP32S3:
            return config_get_xiao_esp32s3_defaults(config);
        case HW_PLATFORM_XIAO_ESP32C3:
            return config_get_xiao_esp32c3_defaults(config);
        case HW_PLATFORM_XIAO_ESP32C6:
            return config_get_xiao_esp32c6_defaults(config);
        case HW_PLATFORM_HELTEC_HT_HC32:
            return config_get_heltec_ht_hc32_defaults(config);
        case HW_PLATFORM_HELTEC_HT_IT01:
            return config_get_heltec_ht_it01_defaults(config);
        case HW_PLATFORM_HELTEC_GENERIC:
            return config_get_heltec_generic_defaults(config);
        default:
            return config_get_xiao_esp32s3_defaults(config); // Default fallback
    }
}

// Platform-specific configuration implementations
bool config_get_xiao_esp32s3_defaults(aircom_config_t* config) {
    config->pin_oled_sda = 5;
    config->pin_oled_scl = 6;
    config->pin_i2s_bclk = 7;
    config->pin_i2s_lrc = 8;
    config->pin_i2s_din = 9;
    config->pin_i2s_dout = 10;
    config->pin_button_ptt = 3;
    config->pin_button_up = 1;
    config->pin_button_down = 2;
    config->pin_button_select = 0;
    config->pin_button_back = 4;
    config->pin_gps_rx = 43;
    config->pin_gps_tx = 44;
    config->pin_led = 21;
    config->pin_battery_adc = 4;

    // Wi-Fi HaLow pins from xiao_esp32_config.h
    config->pin_wifihalow_mosi = get_spi_mosi_pin();
    config->pin_wifihalow_miso = get_spi_miso_pin();
    config->pin_wifihalow_sclk = get_spi_sclk_pin();
    config->pin_wifihalow_cs = get_spi_cs_pin();
    config->pin_wifihalow_reset = get_spi_reset_pin();
    config->pin_wifihalow_int = get_spi_int_pin();

    return true;
}

bool config_get_xiao_esp32c3_defaults(aircom_config_t* config) {
    config->pin_oled_sda = 4;
    config->pin_oled_scl = 5;
    config->pin_i2s_bclk = 6;
    config->pin_i2s_lrc = 7;
    config->pin_i2s_din = 8;
    config->pin_i2s_dout = 9;
    config->pin_button_ptt = 3;
    config->pin_button_up = 1;
    config->pin_button_down = 2;
    config->pin_button_select = 0;
    config->pin_button_back = 10;
    config->pin_gps_rx = 20;
    config->pin_gps_tx = 21;
    config->pin_led = 12;
    config->pin_battery_adc = 2;

    // Wi-Fi HaLow pins from xiao_esp32_config.h
    config->pin_wifihalow_mosi = get_spi_mosi_pin();
    config->pin_wifihalow_miso = get_spi_miso_pin();
    config->pin_wifihalow_sclk = get_spi_sclk_pin();
    config->pin_wifihalow_cs = get_spi_cs_pin();
    config->pin_wifihalow_reset = get_spi_reset_pin();
    config->pin_wifihalow_int = get_spi_int_pin();

    return true;
}

bool config_get_xiao_esp32c6_defaults(aircom_config_t* config) {
    config->pin_oled_sda = 4;
    config->pin_oled_scl = 5;
    config->pin_i2s_bclk = 6;
    config->pin_i2s_lrc = 7;
    config->pin_i2s_din = 8;
    config->pin_i2s_dout = 9;
    config->pin_button_ptt = 3;
    config->pin_button_up = 1;
    config->pin_button_down = 2;
    config->pin_button_select = 0;
    config->pin_button_back = 10;
    config->pin_gps_rx = 20;
    config->pin_gps_tx = 21;
    config->pin_led = 15;
    config->pin_battery_adc = 10;

    // Wi-Fi HaLow pins from xiao_esp32_config.h
    config->pin_wifihalow_mosi = get_spi_mosi_pin();
    config->pin_wifihalow_miso = get_spi_miso_pin();
    config->pin_wifihalow_sclk = get_spi_sclk_pin();
    config->pin_wifihalow_cs = get_spi_cs_pin();
    config->pin_wifihalow_reset = get_spi_reset_pin();
    config->pin_wifihalow_int = get_spi_int_pin();

    return true;
}

bool config_get_heltec_ht_hc32_defaults(aircom_config_t* config) {
    config->pin_oled_sda = 4;
    config->pin_oled_scl = 15;
    config->pin_i2s_bclk = 26;
    config->pin_i2s_lrc = 25;
    config->pin_i2s_din = 33;
    config->pin_i2s_dout = 32;
    config->pin_button_ptt = 12;
    config->pin_button_up = 13;
    config->pin_button_down = 14;
    config->pin_button_select = 0;
    config->pin_button_back = 2;
    config->pin_gps_rx = 34;
    config->pin_gps_tx = 12;
    config->pin_led = 25;
    config->pin_battery_adc = 35;

    // Camera-specific pins (HT-HC32 has camera)
    config->display.enable_touch = false; // No touch on HT-HC32

    // Wi-Fi HaLow pins - using generic ESP32 pins
    config->pin_wifihalow_mosi = 23;
    config->pin_wifihalow_miso = 19;
    config->pin_wifihalow_sclk = 18;
    config->pin_wifihalow_cs = 5;
    config->pin_wifihalow_reset = 17;
    config->pin_wifihalow_int = 16;

    return true;
}

bool config_get_heltec_ht_it01_defaults(aircom_config_t* config) {
    config->pin_oled_sda = 4;
    config->pin_oled_scl = 15;
    config->pin_i2s_bclk = 26;
    config->pin_i2s_lrc = 25;
    config->pin_i2s_din = 33;
    config->pin_i2s_dout = 32;
    config->pin_button_ptt = 12;
    config->pin_button_up = 13;
    config->pin_button_down = 14;
    config->pin_button_select = 0;
    config->pin_button_back = 2;
    config->pin_gps_rx = 34;
    config->pin_gps_tx = 12;
    config->pin_led = 25;
    config->pin_battery_adc = 35;

    // Display-specific configuration (HT-IT01 has display)
    config->display.width = 240;
    config->display.height = 320;
    config->display.enable_touch = true;

    // Wi-Fi HaLow pins - using generic ESP32 pins
    config->pin_wifihalow_mosi = 23;
    config->pin_wifihalow_miso = 19;
    config->pin_wifihalow_sclk = 18;
    config->pin_wifihalow_cs = 5;
    config->pin_wifihalow_reset = 17;
    config->pin_wifihalow_int = 16;

    return true;
}

bool config_get_heltec_generic_defaults(aircom_config_t* config) {
    config->pin_oled_sda = 4;
    config->pin_oled_scl = 15;
    config->pin_i2s_bclk = 26;
    config->pin_i2s_lrc = 25;
    config->pin_i2s_din = 33;
    config->pin_i2s_dout = 32;
    config->pin_button_ptt = 12;
    config->pin_button_up = 13;
    config->pin_button_down = 14;
    config->pin_button_select = 0;
    config->pin_button_back = 2;
    config->pin_gps_rx = 34;
    config->pin_gps_tx = 12;
    config->pin_led = 25;
    config->pin_battery_adc = 35;

    // Wi-Fi HaLow pins - using generic ESP32 pins
    config->pin_wifihalow_mosi = 23;
    config->pin_wifihalow_miso = 19;
    config->pin_wifihalow_sclk = 18;
    config->pin_wifihalow_cs = 5;
    config->pin_wifihalow_reset = 17;
    config->pin_wifihalow_int = 16;

    return true;
}

// ============================================================================
// CONFIGURATION STORAGE FUNCTIONS
// ============================================================================

bool config_manager_init(void) {
    if (g_config_initialized) {
        ESP_LOGW(TAG, "Configuration manager already initialized");
        return true;
    }

    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NVS: %s", esp_err_to_name(err));
        return false;
    }

    // Auto-detect hardware and load defaults
    hardware_platform_t detected_hw = config_manager_detect_hardware();
    ESP_LOGI(TAG, "Detected hardware platform: %s", config_manager_get_platform_name(detected_hw));

    if (!config_manager_get_defaults(detected_hw, &g_current_config)) {
        ESP_LOGE(TAG, "Failed to get default configuration for platform");
        return false;
    }

    // Try to load saved configuration
    if (!config_manager_load(&g_current_config)) {
        ESP_LOGI(TAG, "Using default configuration");
    }

    g_config_initialized = true;
    ESP_LOGI(TAG, "Configuration manager initialized successfully");

    return true;
}

bool config_manager_load(aircom_config_t* config) {
    if (!config) return false;

    nvs_handle_t handle;
    esp_err_t err = nvs_open(CONFIG_NVS_NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "No saved configuration found, using defaults");
        return false;
    }

    // Load network configuration
    size_t str_len = 64;
    char buffer[256];

    if (nvs_get_str(handle, "net.ssid", buffer, &str_len) == ESP_OK) {
        config->network.ssid = buffer;
    }

    if (nvs_get_str(handle, "net.password", buffer, &str_len) == ESP_OK) {
        config->network.password = buffer;
    }

    int32_t int_val;
    if (nvs_get_i32(handle, "net.channel", &int_val) == ESP_OK) {
        config->network.channel = int_val;
    }

    uint8_t bool_val;
    if (nvs_get_u8(handle, "net.enable_mesh", &bool_val) == ESP_OK) {
        config->network.enable_mesh = bool_val;
    }

    nvs_close(handle);
    ESP_LOGI(TAG, "Configuration loaded from NVS");

    return true;
}

bool config_manager_save(const aircom_config_t* config) {
    if (!config) return false;

    nvs_handle_t handle;
    esp_err_t err = nvs_open(CONFIG_NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS for writing: %s", esp_err_to_name(err));
        return false;
    }

    // Save network configuration
    nvs_set_str(handle, "net.ssid", config->network.ssid.c_str());
    nvs_set_str(handle, "net.password", config->network.password.c_str());
    nvs_set_i32(handle, "net.channel", config->network.channel);
    nvs_set_u8(handle, "net.enable_mesh", config->network.enable_mesh);

    err = nvs_commit(handle);
    nvs_close(handle);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to commit configuration to NVS: %s", esp_err_to_name(err));
        return false;
    }

    ESP_LOGI(TAG, "Configuration saved to NVS");
    return true;
}

bool config_manager_reset_to_defaults(void) {
    hardware_platform_t platform = config_manager_detect_hardware();
    return config_manager_get_defaults(platform, &g_current_config);
}

bool config_manager_validate(const aircom_config_t* config) {
    if (!config) return false;

    // Validate network configuration
    if (config->network.ssid.empty()) return false;
    if (config->network.password.length() < 8) return false;
    if (config->network.channel < 1 || config->network.channel > 14) return false;

    // Validate audio configuration
    if (config->audio.sample_rate < 8000 || config->audio.sample_rate > 48000) return false;
    if (config->audio.channels < 1 || config->audio.channels > 2) return false;

    // Validate system configuration
    if (config->system.task_stack_size_default < 1024) return false;

    return true;
}

// ============================================================================
// RUNTIME CONFIGURATION FUNCTIONS
// ============================================================================

bool config_manager_get_string(const char* key, std::string* value) {
    if (!key || !value) return false;

    // Simple key-value lookup (could be extended with a proper key-value store)
    if (strcmp(key, "network.ssid") == 0) {
        *value = g_current_config.network.ssid;
        return true;
    } else if (strcmp(key, "network.password") == 0) {
        *value = g_current_config.network.password;
        return true;
    } else if (strcmp(key, "system.device_name") == 0) {
        *value = g_current_config.system.device_name;
        return true;
    }

    return false;
}

bool config_manager_set_string(const char* key, const std::string& value) {
    if (!key) return false;

    if (strcmp(key, "network.ssid") == 0) {
        g_current_config.network.ssid = value;
        return true;
    } else if (strcmp(key, "network.password") == 0) {
        g_current_config.network.password = value;
        return true;
    } else if (strcmp(key, "system.device_name") == 0) {
        g_current_config.system.device_name = value;
        return true;
    }

    return false;
}

bool config_manager_get_int(const char* key, int* value) {
    if (!key || !value) return false;

    if (strcmp(key, "network.channel") == 0) {
        *value = g_current_config.network.channel;
        return true;
    } else if (strcmp(key, "audio.sample_rate") == 0) {
        *value = g_current_config.audio.sample_rate;
        return true;
    } else if (strcmp(key, "system.log_level") == 0) {
        *value = g_current_config.system.log_level;
        return true;
    }

    return false;
}

bool config_manager_set_int(const char* key, int value) {
    if (!key) return false;

    if (strcmp(key, "network.channel") == 0) {
        g_current_config.network.channel = value;
        return true;
    } else if (strcmp(key, "audio.sample_rate") == 0) {
        g_current_config.audio.sample_rate = value;
        return true;
    } else if (strcmp(key, "system.log_level") == 0) {
        g_current_config.system.log_level = value;
        return true;
    }

    return false;
}

bool config_manager_get_bool(const char* key, bool* value) {
    if (!key || !value) return false;

    if (strcmp(key, "network.enable_mesh") == 0) {
        *value = g_current_config.network.enable_mesh;
        return true;
    } else if (strcmp(key, "audio.enable_compression") == 0) {
        *value = g_current_config.audio.enable_compression;
        return true;
    }

    return false;
}

bool config_manager_set_bool(const char* key, bool value) {
    if (!key) return false;

    if (strcmp(key, "network.enable_mesh") == 0) {
        g_current_config.network.enable_mesh = value;
        return true;
    } else if (strcmp(key, "audio.enable_compression") == 0) {
        g_current_config.audio.enable_compression = value;
        return true;
    }

    return false;
}

void config_manager_print_config(void) {
    ESP_LOGI(TAG, "=== AirCom Configuration ===");
    ESP_LOGI(TAG, "Platform: %s", config_manager_get_platform_name(g_current_config.platform));
    ESP_LOGI(TAG, "Network SSID: %s", g_current_config.network.ssid.c_str());
    ESP_LOGI(TAG, "Network Channel: %u", g_current_config.network.channel);
    ESP_LOGI(TAG, "Audio Sample Rate: %u", g_current_config.audio.sample_rate);
    ESP_LOGI(TAG, "Device Name: %s", g_current_config.system.device_name.c_str());
    ESP_LOGI(TAG, "===========================");
}

// Global accessor for current configuration
const aircom_config_t* config_manager_get_current(void) {
    return g_config_initialized ? &g_current_config : nullptr;
}