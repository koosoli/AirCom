/**
 * @file config_manager.h
 * @brief Unified configuration management system for AirCom
 *
 * This file provides a centralized configuration management system that
 * consolidates all pin definitions, settings, and runtime configuration.
 *
 * @author AirCom Development Team
 * @version 2.0.0
 * @date 2024
 */

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include <string>
#include <map>
#include <vector>
#include "xiao_esp32_config.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// CONFIGURATION DATA TYPES
// ============================================================================

/**
 * @brief Hardware platform enumeration
 */
typedef enum {
    HW_PLATFORM_UNKNOWN = 0,
    HW_PLATFORM_XIAO_ESP32S3,
    HW_PLATFORM_XIAO_ESP32C3,
    HW_PLATFORM_XIAO_ESP32C6,
    HW_PLATFORM_HELTEC_HT_HC32,
    HW_PLATFORM_HELTEC_HT_IT01,
    HW_PLATFORM_HELTEC_GENERIC,
    HW_PLATFORM_ESP32_GENERIC,
    HW_PLATFORM_MAX
} hardware_platform_t;

/**
 * @brief Network configuration
 */
typedef struct {
    std::string ssid;
    std::string password;
    std::string country_code;
    uint32_t channel;
    uint32_t bandwidth;
    bool enable_mesh;
    uint32_t max_connections;
    uint32_t heartbeat_interval;
    uint32_t discovery_timeout;
    bool enable_encryption;
    std::string encryption_key;
    std::vector<std::string> allowed_peer_ids;
} network_config_t;

/**
 * @brief Audio configuration
 */
typedef struct {
    uint32_t sample_rate;
    uint8_t bits_per_sample;
    uint8_t channels;
    uint32_t buffer_size;
    uint32_t queue_depth;
    uint32_t codec_bitrate;
    bool enable_compression;
    bool enable_noise_reduction;
    uint32_t ptt_debounce_ms;
} audio_config_t;

/**
 * @brief Display configuration
 */
typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t rotation;
    bool enable_backlight;
    uint32_t backlight_timeout_ms;
    uint8_t brightness;
    bool enable_touch;
    std::string font_name;
} display_config_t;

/**
 * @brief GPS configuration
 */
typedef struct {
    uint32_t baud_rate;
    uint32_t update_interval_ms;
    bool enable_nmea_output;
    bool enable_debug_output;
    uint32_t fix_timeout_ms;
    float hdop_threshold;
    bool enable_assisted_gps;
} gps_config_t;

/**
 * @brief System configuration
 */
typedef struct {
    uint32_t log_level;
    bool enable_performance_monitoring;
    bool enable_memory_tracking;
    uint32_t watchdog_timeout_ms;
    uint32_t task_stack_size_default;
    uint32_t max_concurrent_connections;
    std::string device_name;
    std::string device_id;
    uint32_t firmware_version;
} system_config_t;

/**
 * @brief Complete configuration structure
 */
typedef struct {
    hardware_platform_t platform;
    network_config_t network;
    audio_config_t audio;
    display_config_t display;
    gps_config_t gps;
    system_config_t system;

    // Hardware-specific pin mappings
    int pin_oled_sda;
    int pin_oled_scl;
    int pin_i2s_bclk;
    int pin_i2s_lrc;
    int pin_i2s_din;
    int pin_i2s_dout;
    int pin_button_ptt;
    int pin_button_up;
    int pin_button_down;
    int pin_button_select;
    int pin_button_back;
    int pin_gps_rx;
    int pin_gps_tx;
    int pin_led;
    int pin_battery_adc;

    // Wi-Fi HaLow specific pins (from xiao_esp32_config.h)
    int pin_wifihalow_mosi;
    int pin_wifihalow_miso;
    int pin_wifihalow_sclk;
    int pin_wifihalow_cs;
    int pin_wifihalow_reset;
    int pin_wifihalow_int;
} aircom_config_t;

// ============================================================================
// CONFIGURATION MANAGER API
// ============================================================================

/**
 * @brief Initialize the configuration management system
 *
 * @return true on success, false on failure
 */
bool config_manager_init(void);

/**
 * @brief Load configuration from storage
 *
 * @param config Pointer to configuration structure to fill
 * @return true on success, false on failure
 */
bool config_manager_load(aircom_config_t* config);

/**
 * @brief Save configuration to storage
 *
 * @param config Configuration to save
 * @return true on success, false on failure
 */
bool config_manager_save(const aircom_config_t* config);

/**
 * @brief Reset configuration to defaults
 *
 * @return true on success, false on failure
 */
bool config_manager_reset_to_defaults(void);

/**
 * @brief Validate configuration integrity
 *
 * @param config Configuration to validate
 * @return true if valid, false if invalid
 */
bool config_manager_validate(const aircom_config_t* config);

/**
 * @brief Auto-detect hardware platform
 *
 * @return Detected hardware platform
 */
hardware_platform_t config_manager_detect_hardware(void);

/**
 * @brief Get default configuration for specific hardware
 *
 * @param platform Hardware platform
 * @param config Output configuration structure
 * @return true on success, false on failure
 */
bool config_manager_get_defaults(hardware_platform_t platform, aircom_config_t* config);

/**
 * @brief Get configuration value by key
 *
 * @param key Configuration key (e.g., "network.ssid")
 * @param value Output string value
 * @return true if found, false if not found
 */
bool config_manager_get_string(const char* key, std::string* value);

/**
 * @brief Set configuration value by key
 *
 * @param key Configuration key
 * @param value String value to set
 * @return true on success, false on failure
 */
bool config_manager_set_string(const char* key, const std::string& value);

/**
 * @brief Get configuration value by key (integer)
 *
 * @param key Configuration key
 * @param value Output integer value
 * @return true if found, false if not found
 */
bool config_manager_get_int(const char* key, int* value);

/**
 * @brief Set configuration value by key (integer)
 *
 * @param key Configuration key
 * @param value Integer value to set
 * @return true on success, false on failure
 */
bool config_manager_set_int(const char* key, int value);

/**
 * @brief Get configuration value by key (boolean)
 *
 * @param key Configuration key
 * @param value Output boolean value
 * @return true if found, false if not found
 */
bool config_manager_get_bool(const char* key, bool* value);

/**
 * @brief Set configuration value by key (boolean)
 *
 * @param key Configuration key
 * @param value Boolean value to set
 * @return true on success, false on failure
 */
bool config_manager_set_bool(const char* key, bool value);

/**
 * @brief Print current configuration (debug)
 */
void config_manager_print_config(void);

/**
 * @brief Get platform name string
 *
 * @param platform Hardware platform
 * @return Platform name string
 */
const char* config_manager_get_platform_name(hardware_platform_t platform);

/**
 * @brief Check if platform is supported
 *
 * @param platform Hardware platform
 * @return true if supported, false otherwise
 */
bool config_manager_is_platform_supported(hardware_platform_t platform);

// ============================================================================
// PLATFORM-SPECIFIC CONFIGURATION FUNCTIONS
// ============================================================================

/**
 * @brief Get XIAO ESP32S3 specific configuration
 *
 * @param config Output configuration
 * @return true on success, false on failure
 */
bool config_get_xiao_esp32s3_defaults(aircom_config_t* config);

/**
 * @brief Get XIAO ESP32C3 specific configuration
 *
 * @param config Output configuration
 * @return true on success, false on failure
 */
bool config_get_xiao_esp32c3_defaults(aircom_config_t* config);

/**
 * @brief Get XIAO ESP32C6 specific configuration
 *
 * @param config Output configuration
 * @return true on success, false on failure
 */
bool config_get_xiao_esp32c6_defaults(aircom_config_t* config);

/**
 * @brief Get Heltec HT-HC32 specific configuration
 *
 * @param config Output configuration
 * @return true on success, false on failure
 */
bool config_get_heltec_ht_hc32_defaults(aircom_config_t* config);

/**
 * @brief Get Heltec HT-IT01 specific configuration
 *
 * @param config Output configuration
 * @return true on success, false on failure
 */
bool config_get_heltec_ht_it01_defaults(aircom_config_t* config);

/**
 * @brief Get generic Heltec configuration
 *
 * @param config Output configuration
 * @return true on success, false on failure
 */
bool config_get_heltec_generic_defaults(aircom_config_t* config);

#ifdef __cplusplus
}
#endif

#endif // CONFIG_MANAGER_H