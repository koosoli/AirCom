/**
 * @file logging_system.cpp
 * @brief Standardized logging and error reporting system implementation
 *
 * This file implements the comprehensive logging system that provides
 * standardized error reporting across all AirCom components.
 *
 * @author AirCom Development Team
 * @version 2.0.0
 * @date 2024
 */

#include "logging_system.h"
#include "esp_log.h"
#include <string>
#include <map>
#include <mutex>
#include <cstdarg>

static const char* TAG = "LOG_SYSTEM";

// Global logging state
static bool g_logging_initialized = false;
static log_level_t g_global_level = LOG_LEVEL_INFO;
static std::map<std::string, log_level_t> g_component_levels;
static std::mutex g_logging_mutex;

// Format string for log messages
static std::string g_log_format = "[%T] %C: %M";

// Output destinations
static bool g_console_output = true;
static bool g_file_output = false;
static bool g_network_output = false;

// Component statistics
static std::map<std::string, std::map<log_level_t, uint32_t>> g_component_stats;

// Helper functions
static uint32_t get_current_timestamp(void) {
    return esp_log_timestamp();
}

static std::string format_log_message(const char* component, log_level_t level,
                                    const char* message, const char* file,
                                    int line, const char* function,
                                    va_list args) {
    char formatted_message[512];
    vsnprintf(formatted_message, sizeof(formatted_message), message, args);

    std::string result = g_log_format;

    // Replace format specifiers
    size_t pos;
    while ((pos = result.find("%T")) != std::string::npos) {
        char timestamp[16];
        snprintf(timestamp, sizeof(timestamp), "%u", get_current_timestamp());
        result.replace(pos, 2, timestamp);
    }

    if ((pos = result.find("%C")) != std::string::npos) {
        result.replace(pos, 2, component);
    }

    if ((pos = result.find("%M")) != std::string::npos) {
        result.replace(pos, 2, formatted_message);
    }

    if ((pos = result.find("%F")) != std::string::npos) {
        result.replace(pos, 2, file);
    }

    if ((pos = result.find("%L")) != std::string::npos) {
        char line_str[16];
        snprintf(line_str, sizeof(line_str), "%d", line);
        result.replace(pos, 2, line_str);
    }

    if ((pos = result.find("%U")) != std::string::npos) {
        result.replace(pos, 2, function);
    }

    return result;
}

static const char* log_level_to_string(log_level_t level) {
    switch (level) {
        case LOG_LEVEL_ERROR: return "ERROR";
        case LOG_LEVEL_WARNING: return "WARN";
        case LOG_LEVEL_INFO: return "INFO";
        case LOG_LEVEL_DEBUG: return "DEBUG";
        case LOG_LEVEL_VERBOSE: return "VERBOSE";
        default: return "UNKNOWN";
    }
}

// Public API implementation
bool logging_system_init(log_level_t default_level) {
    if (g_logging_initialized) {
        ESP_LOGW(TAG, "Logging system already initialized");
        return true;
    }

    std::lock_guard<std::mutex> lock(g_logging_mutex);

    g_global_level = default_level;
    g_logging_initialized = true;

    ESP_LOGI(TAG, "Logging system initialized with level %d", default_level);
    return true;
}

void logging_system_set_component_level(const char* component, log_level_t level) {
    if (!g_logging_initialized || !component) return;

    std::lock_guard<std::mutex> lock(g_logging_mutex);
    g_component_levels[component] = level;
}

void logging_system_set_global_level(log_level_t level) {
    if (!g_logging_initialized) return;

    std::lock_guard<std::mutex> lock(g_logging_mutex);
    g_global_level = level;
}

log_level_t logging_system_get_component_level(const char* component) {
    if (!g_logging_initialized || !component) return g_global_level;

    std::lock_guard<std::mutex> lock(g_logging_mutex);
    auto it = g_component_levels.find(component);
    return (it != g_component_levels.end()) ? it->second : g_global_level;
}

bool logging_system_is_enabled(const char* component, log_level_t level) {
    if (!g_logging_initialized) return false;

    log_level_t component_level = logging_system_get_component_level(component);
    return level <= component_level;
}

// Logging functions
void logging_system_log_error(const char* component, error_code_t code, const char* message,
                             const char* file, int line, const char* function, ...) {
    if (!logging_system_is_enabled(component, LOG_LEVEL_ERROR)) return;

    std::lock_guard<std::mutex> lock(g_logging_mutex);

    va_list args;
    va_start(args, function);
    std::string formatted = format_log_message(component, LOG_LEVEL_ERROR, message,
                                             file, line, function, args);
    va_end(args);

    // Output to console
    if (g_console_output) {
        ESP_LOGE(component, "%s", formatted.c_str());
    }

    // Update statistics
    g_component_stats[component][LOG_LEVEL_ERROR]++;
}

void logging_system_log_warning(const char* component, const char* message,
                               const char* file, int line, const char* function, ...) {
    if (!logging_system_is_enabled(component, LOG_LEVEL_WARNING)) return;

    std::lock_guard<std::mutex> lock(g_logging_mutex);

    va_list args;
    va_start(args, function);
    std::string formatted = format_log_message(component, LOG_LEVEL_WARNING, message,
                                             file, line, function, args);
    va_end(args);

    if (g_console_output) {
        ESP_LOGW(component, "%s", formatted.c_str());
    }

    g_component_stats[component][LOG_LEVEL_WARNING]++;
}

void logging_system_log_info(const char* component, const char* message,
                            const char* file, int line, const char* function, ...) {
    if (!logging_system_is_enabled(component, LOG_LEVEL_INFO)) return;

    std::lock_guard<std::mutex> lock(g_logging_mutex);

    va_list args;
    va_start(args, function);
    std::string formatted = format_log_message(component, LOG_LEVEL_INFO, message,
                                             file, line, function, args);
    va_end(args);

    if (g_console_output) {
        ESP_LOGI(component, "%s", formatted.c_str());
    }

    g_component_stats[component][LOG_LEVEL_INFO]++;
}

void logging_system_log_debug(const char* component, const char* message,
                             const char* file, int line, const char* function, ...) {
    if (!logging_system_is_enabled(component, LOG_LEVEL_DEBUG)) return;

    std::lock_guard<std::mutex> lock(g_logging_mutex);

    va_list args;
    va_start(args, function);
    std::string formatted = format_log_message(component, LOG_LEVEL_DEBUG, message,
                                             file, line, function, args);
    va_end(args);

    if (g_console_output) {
        ESP_LOGD(component, "%s", formatted.c_str());
    }

    g_component_stats[component][LOG_LEVEL_DEBUG]++;
}

void logging_system_log_verbose(const char* component, const char* message,
                               const char* file, int line, const char* function, ...) {
    if (!logging_system_is_enabled(component, LOG_LEVEL_VERBOSE)) return;

    std::lock_guard<std::mutex> lock(g_logging_mutex);

    va_list args;
    va_start(args, function);
    std::string formatted = format_log_message(component, LOG_LEVEL_VERBOSE, message,
                                             file, line, function, args);
    va_end(args);

    if (g_console_output) {
        ESP_LOGV(component, "%s", formatted.c_str());
    }

    g_component_stats[component][LOG_LEVEL_VERBOSE]++;
}

// Output control functions
void logging_system_set_console_output(bool enable) {
    std::lock_guard<std::mutex> lock(g_logging_mutex);
    g_console_output = enable;
}

void logging_system_set_file_output(bool enable, size_t max_file_size, uint32_t max_files) {
    std::lock_guard<std::mutex> lock(g_logging_mutex);
    g_file_output = enable;
    // TODO: Implement file output
}

void logging_system_set_network_output(bool enable, const char* host, uint16_t port) {
    std::lock_guard<std::mutex> lock(g_logging_mutex);
    g_network_output = enable;
    // TODO: Implement network output
}

// Statistics and monitoring functions
bool logging_system_get_component_stats(const char* component,
                                       uint32_t* error_count,
                                       uint32_t* warning_count,
                                       uint32_t* info_count) {
    if (!component || !error_count || !warning_count || !info_count) return false;

    std::lock_guard<std::mutex> lock(g_logging_mutex);

    auto component_it = g_component_stats.find(component);
    if (component_it == g_component_stats.end()) {
        *error_count = 0;
        *warning_count = 0;
        *info_count = 0;
        return true;
    }

    auto& stats = component_it->second;
    *error_count = stats[LOG_LEVEL_ERROR];
    *warning_count = stats[LOG_LEVEL_WARNING];
    *info_count = stats[LOG_LEVEL_INFO];

    return true;
}

void logging_system_reset_component_stats(const char* component) {
    if (!component) return;

    std::lock_guard<std::mutex> lock(g_logging_mutex);
    g_component_stats.erase(component);
}

bool logging_system_check_error_threshold(const char* component, uint32_t max_errors) {
    uint32_t error_count = 0;
    uint32_t warning_count = 0;
    uint32_t info_count = 0;

    if (!logging_system_get_component_stats(component, &error_count, &warning_count, &info_count)) {
        return false;
    }

    return error_count > max_errors;
}

// Configuration functions
void logging_system_set_format(const char* format) {
    if (!format) return;

    std::lock_guard<std::mutex> lock(g_logging_mutex);
    g_log_format = format;
}

void logging_system_add_filter(const char* component, log_level_t min_level, log_level_t max_level) {
    // TODO: Implement filtering system
}

void logging_system_clear_filters(void) {
    // TODO: Clear all filters
}