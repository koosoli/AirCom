#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

// ============================================================================
// ERROR HANDLING FRAMEWORK - PRODUCTION READY IMPLEMENTATION
//
// This module provides a centralized error handling system for the AirCom system.
// Features:
// - Standardized error codes and categories
// - Error recovery strategies
// - Error logging with context
// - Retry mechanisms with exponential backoff
// - Error statistics and monitoring
// - Thread-safe error state management
//
// Usage:
// 1. Use ERROR_CHECK() macro for error checking
// 2. Use RETRY_WITH_BACKOFF() for retry logic
// 3. Use ERROR_LOG() for contextual error logging
// 4. Call error_handling_init() at startup
// ============================================================================

#include <stdint.h>
#include <stdbool.h>
#include <esp_err.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// ERROR CODES AND CATEGORIES
// ============================================================================

// Error categories
typedef enum {
    ERROR_CATEGORY_NONE = 0,
    ERROR_CATEGORY_SYSTEM,        // System-level errors (memory, OS, etc.)
    ERROR_CATEGORY_NETWORK,       // Network communication errors
    ERROR_CATEGORY_AUDIO,         // Audio processing errors
    ERROR_CATEGORY_CRYPTO,        // Cryptographic operation errors
    ERROR_CATEGORY_STORAGE,       // Storage/Flash errors
    ERROR_CATEGORY_SENSOR,        // Sensor reading errors
    ERROR_CATEGORY_UI,            // User interface errors
    ERROR_CATEGORY_CONFIG,        // Configuration errors
    ERROR_CATEGORY_HARDWARE,      // Hardware interface errors
    ERROR_CATEGORY_MAX
} error_category_t;

// Specific error codes
typedef enum {
    ERROR_NONE = 0,

    // System errors (1000-1999)
    ERROR_MEMORY_ALLOCATION = 1001,
    ERROR_NULL_POINTER = 1002,
    ERROR_INVALID_PARAMETER = 1003,
    ERROR_TIMEOUT = 1004,
    ERROR_RESOURCE_UNAVAILABLE = 1005,
    ERROR_SYSTEM_OVERLOAD = 1006,
    ERROR_TASK_CREATION = 1007,

    // Network errors (2000-2999)
    ERROR_SOCKET_CREATE = 2001,
    ERROR_SOCKET_BIND = 2002,
    ERROR_SOCKET_CONNECT = 2003,
    ERROR_SOCKET_SEND = 2004,
    ERROR_SOCKET_RECEIVE = 2005,
    ERROR_NETWORK_TIMEOUT = 2006,
    ERROR_INVALID_ADDRESS = 2007,
    ERROR_CONNECTION_LOST = 2008,

    // Audio errors (3000-3999)
    ERROR_AUDIO_INIT = 3001,
    ERROR_AUDIO_CONFIG = 3002,
    ERROR_AUDIO_ENCODE = 3003,
    ERROR_AUDIO_DECODE = 3004,
    ERROR_AUDIO_BUFFER_OVERFLOW = 3005,
    ERROR_AUDIO_BUFFER_UNDERFLOW = 3006,
    ERROR_AUDIO_DEVICE = 3007,

    // Crypto errors (4000-4999)
    ERROR_CRYPTO_INIT = 4001,
    ERROR_CRYPTO_KEY = 4002,
    ERROR_CRYPTO_ENCRYPT = 4003,
    ERROR_CRYPTO_DECRYPT = 4004,
    ERROR_CRYPTO_AUTH = 4005,

    // Storage errors (5000-5999)
    ERROR_STORAGE_INIT = 5001,
    ERROR_STORAGE_READ = 5002,
    ERROR_STORAGE_WRITE = 5003,
    ERROR_STORAGE_ERASE = 5004,
    ERROR_STORAGE_FULL = 5005,
    ERROR_NVS_INIT = 5006,

    // Sensor errors (6000-6999)
    ERROR_SENSOR_INIT = 6001,
    ERROR_SENSOR_READ = 6002,
    ERROR_SENSOR_CALIBRATION = 6003,
    ERROR_SENSOR_TIMEOUT = 6004,

    // UI errors (7000-7999)
    ERROR_UI_INIT = 7001,
    ERROR_UI_UPDATE = 7002,
    ERROR_UI_INPUT = 7003,
    ERROR_UI_MEMORY = 7004,

    // Configuration errors (8000-8999)
    ERROR_CONFIG_INVALID = 8001,
    ERROR_CONFIG_MISSING = 8002,
    ERROR_CONFIG_CORRUPT = 8003,

    // Hardware errors (9000-9999)
    ERROR_HARDWARE_INIT = 9001,
    ERROR_HARDWARE_ACCESS = 9002,
    ERROR_HARDWARE_TIMEOUT = 9003,
    ERROR_HARDWARE_FAULT = 9004,
} error_code_t;

// ============================================================================
// ERROR CONTEXT AND RECOVERY
// ============================================================================

// Error context structure
typedef struct {
    error_category_t category;
    error_code_t code;
    const char* message;
    const char* file;
    int line;
    const char* function;
    uint32_t timestamp;
    void* context_data;
    size_t context_size;
} error_context_t;

// Recovery strategy types
typedef enum {
    RECOVERY_NONE,           // No recovery possible
    RECOVERY_RETRY,          // Simple retry
    RECOVERY_BACKOFF,        // Retry with exponential backoff
    RECOVERY_RESET,          // Reset and reinitialize
    RECOVERY_FALLBACK,       // Use fallback mechanism
    RECOVERY_RESTART,        // Restart the component
    RECOVERY_ESCALATE        // Escalate to higher level handler
} recovery_strategy_t;

// Error statistics
typedef struct {
    uint32_t total_errors;
    uint32_t errors_by_category[ERROR_CATEGORY_MAX];
    uint32_t recovery_attempts;
    uint32_t successful_recoveries;
    uint32_t failed_recoveries;
    uint32_t last_error_timestamp;
    error_code_t last_error_code;
} error_stats_t;

// ============================================================================
// ERROR HANDLING API
// ============================================================================

/**
 * @brief Initialize the error handling system
 *
 * @return true on success, false on failure
 */
bool error_handling_init(void);

/**
 * @brief Report an error with context
 *
 * @param category Error category
 * @param code Error code
 * @param message Error message
 * @param file Source file where error occurred
 * @param line Line number where error occurred
 * @param function Function name where error occurred
 * @param context_data Additional context data (can be NULL)
 * @param context_size Size of context data
 * @return Error context structure
 */
error_context_t error_report(error_category_t category, error_code_t code,
                           const char* message, const char* file, int line,
                           const char* function, void* context_data, size_t context_size);

/**
 * @brief Attempt error recovery
 *
 * @param error Error context
 * @return true if recovery successful, false otherwise
 */
bool error_recover(const error_context_t* error);

/**
 * @brief Get error statistics
 *
 * @param stats Pointer to store statistics
 * @return true on success, false on failure
 */
bool error_get_stats(error_stats_t* stats);

/**
 * @brief Reset error statistics
 */
void error_reset_stats(void);

/**
 * @brief Check if system is in error state
 *
 * @return true if in error state, false otherwise
 */
bool error_is_in_error_state(void);

/**
 * @brief Clear error state
 */
void error_clear_state(void);

// ============================================================================
// RETRY MECHANISMS
// ============================================================================

// Retry configuration
typedef struct {
    uint32_t max_attempts;
    uint32_t base_delay_ms;
    uint32_t max_delay_ms;
    float backoff_multiplier;
    bool jitter_enabled;
} retry_config_t;

// Default retry configurations
extern const retry_config_t RETRY_CONFIG_DEFAULT;
extern const retry_config_t RETRY_CONFIG_NETWORK;
extern const retry_config_t RETRY_CONFIG_AUDIO;
extern const retry_config_t RETRY_CONFIG_STORAGE;

/**
 * @brief Execute function with retry logic
 *
 * @param func Function to execute (should return true on success)
 * @param context Context data to pass to function
 * @param config Retry configuration
 * @param error_context Error context for reporting (can be NULL)
 * @return true on success, false on failure after all retries
 */
bool retry_with_backoff(bool (*func)(void*), void* context,
                       const retry_config_t* config, error_context_t* error_context);

/**
 * @brief Calculate delay for exponential backoff with jitter
 *
 * @param attempt Current attempt number (0-based)
 * @param base_delay Base delay in milliseconds
 * @param max_delay Maximum delay in milliseconds
 * @param multiplier Backoff multiplier
 * @param jitter_enabled Enable random jitter
 * @return Delay in milliseconds
 */
uint32_t calculate_backoff_delay(uint32_t attempt, uint32_t base_delay,
                                uint32_t max_delay, float multiplier, bool jitter_enabled);

// ============================================================================
// ERROR HANDLING MACROS
// ============================================================================

// Error checking macro
#define ERROR_CHECK(condition, category, code, message) \
    do { \
        if (!(condition)) { \
            error_report((category), (code), (message), __FILE__, __LINE__, __func__, NULL, 0); \
        } \
    } while (0)

// Error checking with custom context
#define ERROR_CHECK_CTX(condition, category, code, message, context, size) \
    do { \
        if (!(condition)) { \
            error_report((category), (code), (message), __FILE__, __LINE__, __func__, (context), (size)); \
        } \
    } while (0)

// ESP-IDF error conversion
#define ESP_ERROR_TO_AIRCOM(err) \
    ((err) == ESP_OK ? ERROR_NONE : \
     (err) == ESP_ERR_NO_MEM ? ERROR_MEMORY_ALLOCATION : \
     (err) == ESP_ERR_TIMEOUT ? ERROR_TIMEOUT : \
     ERROR_SYSTEM_OVERLOAD)

// Error logging macro
#define ERROR_LOG(category, code, message) \
    error_report((category), (code), (message), __FILE__, __LINE__, __func__, NULL, 0)

// Retry macro with default configuration
#define RETRY_WITH_BACKOFF(func, context) \
    retry_with_backoff((func), (context), &RETRY_CONFIG_DEFAULT, NULL)

// Retry macro with custom configuration
#define RETRY_WITH_BACKOFF_CFG(func, context, config) \
    retry_with_backoff((func), (context), (config), NULL)

#ifdef __cplusplus
}
#endif

#endif // ERROR_HANDLING_H