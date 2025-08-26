/**
 * @file logging_system.h
 * @brief Standardized logging and error reporting system for AirCom
 *
 * This file provides a comprehensive logging system that standardizes error
 * reporting across all components and replaces inconsistent ESP_LOGX usage.
 *
 * @author AirCom Development Team
 * @version 2.0.0
 * @date 2024
 */

#ifndef LOGGING_SYSTEM_H
#define LOGGING_SYSTEM_H

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include "error_handling.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// LOG LEVEL DEFINITIONS
// ============================================================================

typedef enum {
    LOG_LEVEL_NONE = 0,     // No logging
    LOG_LEVEL_ERROR,        // Critical errors only
    LOG_LEVEL_WARNING,      // Errors and warnings
    LOG_LEVEL_INFO,         // General information
    LOG_LEVEL_DEBUG,        // Debug information
    LOG_LEVEL_VERBOSE,      // Very detailed information
    LOG_LEVEL_MAX
} log_level_t;

// ============================================================================
// LOG CONTEXT DEFINITION
// ============================================================================

typedef struct {
    const char* component;      // Component name (e.g., "NETWORK", "AUDIO")
    const char* file;           // Source file name
    int line;                   // Line number
    const char* function;       // Function name
    uint32_t timestamp;         // Timestamp
    uint32_t thread_id;         // Thread ID
    error_category_t category;  // Error category
    error_code_t code;          // Error code
} log_context_t;

// ============================================================================
// LOGGING SYSTEM API
// ============================================================================

/**
 * @brief Initialize the logging system
 *
 * @param default_level Default log level
 * @return true on success, false on failure
 */
bool logging_system_init(log_level_t default_level);

/**
 * @brief Set log level for specific component
 *
 * @param component Component name
 * @param level Log level
 */
void logging_system_set_component_level(const char* component, log_level_t level);

/**
 * @brief Set global log level
 *
 * @param level Global log level
 */
void logging_system_set_global_level(log_level_t level);

/**
 * @brief Get current log level for component
 *
 * @param component Component name
 * @return Current log level
 */
log_level_t logging_system_get_component_level(const char* component);

/**
 * @brief Check if logging is enabled for level and component
 *
 * @param component Component name
 * @param level Log level to check
 * @return true if logging is enabled, false otherwise
 */
bool logging_system_is_enabled(const char* component, log_level_t level);

// ============================================================================
// STANDARDIZED LOGGING MACROS
// ============================================================================

// Error logging with error context
#define LOG_ERROR(component, error_code, message, ...) \
    logging_system_log_error(component, (error_code), (message), __FILE__, __LINE__, __func__, ##__VA_ARGS__)

// Warning logging
#define LOG_WARNING(component, message, ...) \
    logging_system_log_warning(component, (message), __FILE__, __LINE__, __func__, ##__VA_ARGS__)

// Info logging
#define LOG_INFO(component, message, ...) \
    logging_system_log_info(component, (message), __FILE__, __LINE__, __func__, ##__VA_ARGS__)

// Debug logging
#define LOG_DEBUG(component, message, ...) \
    logging_system_log_debug(component, (message), __FILE__, __LINE__, __func__, ##__VA_ARGS__)

// Verbose logging
#define LOG_VERBOSE(component, message, ...) \
    logging_system_log_verbose(component, (message), __FILE__, __LINE__, __func__, ##__VA_ARGS__)

// Component-specific macros for common components
#define LOG_NETWORK_ERROR(code, msg, ...) LOG_ERROR("NETWORK", (code), (msg), ##__VA_ARGS__)
#define LOG_NETWORK_WARNING(msg, ...) LOG_WARNING("NETWORK", (msg), ##__VA_ARGS__)
#define LOG_NETWORK_INFO(msg, ...) LOG_INFO("NETWORK", (msg), ##__VA_ARGS__)
#define LOG_NETWORK_DEBUG(msg, ...) LOG_DEBUG("NETWORK", (msg), ##__VA_ARGS__)

#define LOG_AUDIO_ERROR(code, msg, ...) LOG_ERROR("AUDIO", (code), (msg), ##__VA_ARGS__)
#define LOG_AUDIO_WARNING(msg, ...) LOG_WARNING("AUDIO", (msg), ##__VA_ARGS__)
#define LOG_AUDIO_INFO(msg, ...) LOG_INFO("AUDIO", (msg), ##__VA_ARGS__)
#define LOG_AUDIO_DEBUG(msg, ...) LOG_DEBUG("AUDIO", (msg), ##__VA_ARGS__)

#define LOG_UI_ERROR(code, msg, ...) LOG_ERROR("UI", (code), (msg), ##__VA_ARGS__)
#define LOG_UI_WARNING(msg, ...) LOG_WARNING("UI", (msg), ##__VA_ARGS__)
#define LOG_UI_INFO(msg, ...) LOG_INFO("UI", (msg), ##__VA_ARGS__)
#define LOG_UI_DEBUG(msg, ...) LOG_DEBUG("UI", (msg), ##__VA_ARGS__)

#define LOG_SYSTEM_ERROR(code, msg, ...) LOG_ERROR("SYSTEM", (code), (msg), ##__VA_ARGS__)
#define LOG_SYSTEM_WARNING(msg, ...) LOG_WARNING("SYSTEM", (msg), ##__VA_ARGS__)
#define LOG_SYSTEM_INFO(msg, ...) LOG_INFO("SYSTEM", (msg), ##__VA_ARGS__)
#define LOG_SYSTEM_DEBUG(msg, ...) LOG_DEBUG("SYSTEM", (msg), ##__VA_ARGS__)

// ============================================================================
// LOGGING FUNCTIONS
// ============================================================================

/**
 * @brief Log error with error context
 *
 * @param component Component name
 * @param code Error code
 * @param message Error message
 * @param file Source file
 * @param line Line number
 * @param function Function name
 * @param ... Additional format arguments
 */
void logging_system_log_error(const char* component, error_code_t code, const char* message,
                             const char* file, int line, const char* function, ...);

/**
 * @brief Log warning message
 *
 * @param component Component name
 * @param message Warning message
 * @param file Source file
 * @param line Line number
 * @param function Function name
 * @param ... Additional format arguments
 */
void logging_system_log_warning(const char* component, const char* message,
                               const char* file, int line, const char* function, ...);

/**
 * @brief Log info message
 *
 * @param component Component name
 * @param message Info message
 * @param file Source file
 * @param line Line number
 * @param function Function name
 * @param ... Additional format arguments
 */
void logging_system_log_info(const char* component, const char* message,
                            const char* file, int line, const char* function, ...);

/**
 * @brief Log debug message
 *
 * @param component Component name
 * @param message Debug message
 * @param file Source file
 * @param line Line number
 * @param function Function name
 * @param ... Additional format arguments
 */
void logging_system_log_debug(const char* component, const char* message,
                             const char* file, int line, const char* function, ...);

/**
 * @brief Log verbose message
 *
 * @param component Component name
 * @param message Verbose message
 * @param file Source file
 * @param line Line number
 * @param function Function name
 * @param ... Additional format arguments
 */
void logging_system_log_verbose(const char* component, const char* message,
                               const char* file, int line, const char* function, ...);

// ============================================================================
// LOG OUTPUT CONTROL
// ============================================================================

/**
 * @brief Enable/disable logging to console
 *
 * @param enable true to enable, false to disable
 */
void logging_system_set_console_output(bool enable);

/**
 * @brief Enable/disable logging to file system
 *
 * @param enable true to enable, false to disable
 * @param max_file_size Maximum file size in bytes
 * @param max_files Maximum number of log files to keep
 */
void logging_system_set_file_output(bool enable, size_t max_file_size, uint32_t max_files);

/**
 * @brief Enable/disable logging to network
 *
 * @param enable true to enable, false to disable
 * @param host Remote host IP
 * @param port Remote port
 */
void logging_system_set_network_output(bool enable, const char* host, uint16_t port);

// ============================================================================
// LOG ANALYSIS AND MONITORING
// ============================================================================

/**
 * @brief Get log statistics for component
 *
 * @param component Component name
 * @param error_count Output error count
 * @param warning_count Output warning count
 * @param info_count Output info count
 * @return true on success, false on failure
 */
bool logging_system_get_component_stats(const char* component,
                                       uint32_t* error_count,
                                       uint32_t* warning_count,
                                       uint32_t* info_count);

/**
 * @brief Reset log statistics for component
 *
 * @param component Component name
 */
void logging_system_reset_component_stats(const char* component);

/**
 * @brief Check if component has exceeded error threshold
 *
 * @param component Component name
 * @param max_errors Maximum allowed errors
 * @return true if threshold exceeded, false otherwise
 */
bool logging_system_check_error_threshold(const char* component, uint32_t max_errors);

// ============================================================================
// LOG FORMAT AND FILTERING
// ============================================================================

/**
 * @brief Set log message format
 *
 * @param format Format string (e.g., "[%T] %C: %M")
 *                %T = Timestamp
 *                %C = Component
 *                %F = File
 *                %L = Line
 *                %U = Function
 *                %M = Message
 *                %E = Error code
 */
void logging_system_set_format(const char* format);

/**
 * @brief Add log filter
 *
 * @param component Component to filter (NULL for all)
 * @param min_level Minimum log level to include
 * @param max_level Maximum log level to include
 */
void logging_system_add_filter(const char* component, log_level_t min_level, log_level_t max_level);

/**
 * @brief Clear all log filters
 */
void logging_system_clear_filters(void);

// ============================================================================
// BACKWARD COMPATIBILITY MACROS
// ============================================================================

// These macros provide backward compatibility with existing ESP_LOGX calls
// They automatically map to the new logging system

#undef ESP_LOGE
#define ESP_LOGE(tag, format, ...) LOG_ERROR(tag, ERROR_NONE, format, ##__VA_ARGS__)
#undef ESP_LOGW
#define ESP_LOGW(tag, format, ...) LOG_WARNING(tag, format, ##__VA_ARGS__)
#undef ESP_LOGI
#define ESP_LOGI(tag, format, ...) LOG_INFO(tag, format, ##__VA_ARGS__)
#undef ESP_LOGD
#define ESP_LOGD(tag, format, ...) LOG_DEBUG(tag, format, ##__VA_ARGS__)
#undef ESP_LOGV
#define ESP_LOGV(tag, format, ...) LOG_VERBOSE(tag, format, ##__VA_ARGS__)

// Error context logging macros
#define LOG_ERROR_CTX(component, category, code, message, context) \
    do { \
        error_context_t err_ctx = error_report((category), (code), (message), __FILE__, __LINE__, __func__, (context), sizeof(*(context))); \
        logging_system_log_error(component, (code), message, __FILE__, __LINE__, __func__); \
    } while (0)

// Performance logging macros
#define LOG_PERF_START(component, operation) \
    uint32_t perf_start_##operation = esp_log_timestamp(); \
    LOG_DEBUG(component, "Starting operation: %s", #operation)

#define LOG_PERF_END(component, operation) \
    uint32_t perf_end_##operation = esp_log_timestamp(); \
    LOG_INFO(component, "Operation %s completed in %u ms", #operation, (perf_end_##operation - perf_start_##operation))

#ifdef __cplusplus
}
#endif

#endif // LOGGING_SYSTEM_H