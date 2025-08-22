#include "include/error_handling.h"
#include <string.h>
#include <stdlib.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

static const char* TAG = "ERROR_HANDLING";

// ============================================================================
// INTERNAL STATE
// ============================================================================

static bool g_initialized = false;
static error_stats_t g_stats = {0};
static SemaphoreHandle_t g_stats_mutex = NULL;

// Default retry configurations
const retry_config_t RETRY_CONFIG_DEFAULT = {
    .max_attempts = 3,
    .base_delay_ms = 100,
    .max_delay_ms = 5000,
    .backoff_multiplier = 2.0f,
    .jitter_enabled = true
};

const retry_config_t RETRY_CONFIG_NETWORK = {
    .max_attempts = 5,
    .base_delay_ms = 500,
    .max_delay_ms = 30000,
    .backoff_multiplier = 1.5f,
    .jitter_enabled = true
};

const retry_config_t RETRY_CONFIG_AUDIO = {
    .max_attempts = 3,
    .base_delay_ms = 50,
    .max_delay_ms = 1000,
    .backoff_multiplier = 2.0f,
    .jitter_enabled = false
};

const retry_config_t RETRY_CONFIG_STORAGE = {
    .max_attempts = 3,
    .base_delay_ms = 1000,
    .max_delay_ms = 10000,
    .backoff_multiplier = 2.0f,
    .jitter_enabled = true
};

// ============================================================================
// INTERNAL HELPER FUNCTIONS
// ============================================================================

/**
 * @brief Get current timestamp in milliseconds
 */
static uint32_t get_timestamp_ms(void) {
    return (uint32_t)(esp_timer_get_time() / 1000);
}

/**
 * @brief Update error statistics (thread-safe)
 */
static void update_stats(error_category_t category, error_code_t code) {
    if (!g_initialized || !g_stats_mutex) {
        return;
    }

    if (xSemaphoreTake(g_stats_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        g_stats.total_errors++;
        if (category < ERROR_CATEGORY_MAX) {
            g_stats.errors_by_category[category]++;
        }
        g_stats.last_error_code = code;
        g_stats.last_error_timestamp = get_timestamp_ms();
        xSemaphoreGive(g_stats_mutex);
    }
}

/**
 * @brief Get recovery strategy for error
 */
static recovery_strategy_t get_recovery_strategy(const error_context_t* error) {
    switch (error->category) {
        case ERROR_CATEGORY_NETWORK:
            return RECOVERY_BACKOFF;

        case ERROR_CATEGORY_AUDIO:
            return RECOVERY_RETRY;

        case ERROR_CATEGORY_SYSTEM:
            if (error->code == ERROR_MEMORY_ALLOCATION) {
                return RECOVERY_RESET;
            } else if (error->code == ERROR_TIMEOUT) {
                return RECOVERY_BACKOFF;
            }
            return RECOVERY_RETRY;

        case ERROR_CATEGORY_HARDWARE:
            return RECOVERY_RESET;

        case ERROR_CATEGORY_STORAGE:
            return RECOVERY_BACKOFF;

        default:
            return RECOVERY_RETRY;
    }
}

/**
 * @brief Attempt recovery based on strategy
 */
static bool attempt_recovery(const error_context_t* error, recovery_strategy_t strategy) {
    bool recovered = false;

    switch (strategy) {
        case RECOVERY_NONE:
            recovered = false;
            break;

        case RECOVERY_RETRY:
            // Simple retry - assume the calling code will handle it
            recovered = true;
            break;

        case RECOVERY_BACKOFF:
            // Backoff handled by retry mechanism
            recovered = true;
            break;

        case RECOVERY_RESET:
            // Reset would require system-specific reset logic
            ESP_LOGI(TAG, "Reset recovery requested for error %d", error->code);
            recovered = false; // Not implemented in this generic handler
            break;

        case RECOVERY_FALLBACK:
            ESP_LOGI(TAG, "Fallback recovery requested for error %d", error->code);
            recovered = true; // Assume fallback is available
            break;

        case RECOVERY_RESTART:
            ESP_LOGI(TAG, "Restart recovery requested for error %d", error->code);
            recovered = false; // Not implemented in this generic handler
            break;

        case RECOVERY_ESCALATE:
            ESP_LOGW(TAG, "Escalating error %d to higher level", error->code);
            recovered = false;
            break;

        default:
            recovered = false;
            break;
    }

    return recovered;
}

/**
 * @brief Add random jitter to delay
 */
static uint32_t add_jitter(uint32_t delay_ms, uint32_t max_jitter_ms) {
    if (max_jitter_ms == 0) {
        return delay_ms;
    }

    uint32_t jitter = rand() % max_jitter_ms;
    return delay_ms + jitter;
}

// ============================================================================
// PUBLIC API IMPLEMENTATION
// ============================================================================

bool error_handling_init(void) {
    if (g_initialized) {
        ESP_LOGW(TAG, "Error handling already initialized");
        return true;
    }

    // Create mutex for thread-safe statistics
    g_stats_mutex = xSemaphoreCreateMutex();
    if (!g_stats_mutex) {
        ESP_LOGE(TAG, "Failed to create statistics mutex");
        return false;
    }

    // Initialize statistics
    memset(&g_stats, 0, sizeof(g_stats));

    g_initialized = true;
    ESP_LOGI(TAG, "Error handling system initialized");
    return true;
}

error_context_t error_report(error_category_t category, error_code_t code,
                           const char* message, const char* file, int line,
                           const char* function, void* context_data, size_t context_size) {
    error_context_t error = {
        .category = category,
        .code = code,
        .message = message,
        .file = file,
        .line = line,
        .function = function,
        .timestamp = get_timestamp_ms(),
        .context_data = context_data,
        .context_size = context_size
    };

    // Update statistics
    update_stats(category, code);

    // Log error with context
    ESP_LOGE(TAG, "ERROR [%d] %s:%d in %s(): %s (code: %d)",
             category, file, line, function, message, code);

    if (context_data && context_size > 0) {
        ESP_LOG_BUFFER_HEXDUMP(TAG, context_data, context_size, ESP_LOG_DEBUG);
    }

    return error;
}

bool error_recover(const error_context_t* error) {
    if (!error) {
        return false;
    }

    recovery_strategy_t strategy = get_recovery_strategy(error);
    bool recovered = attempt_recovery(error, strategy);

    // Update recovery statistics
    if (g_stats_mutex && xSemaphoreTake(g_stats_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        g_stats.recovery_attempts++;
        if (recovered) {
            g_stats.successful_recoveries++;
        } else {
            g_stats.failed_recoveries++;
        }
        xSemaphoreGive(g_stats_mutex);
    }

    ESP_LOGI(TAG, "Recovery %s for error %d (strategy: %d)",
             recovered ? "successful" : "failed", error->code, strategy);

    return recovered;
}

bool error_get_stats(error_stats_t* stats) {
    if (!g_initialized || !stats || !g_stats_mutex) {
        return false;
    }

    if (xSemaphoreTake(g_stats_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        memcpy(stats, &g_stats, sizeof(error_stats_t));
        xSemaphoreGive(g_stats_mutex);
        return true;
    }

    return false;
}

void error_reset_stats(void) {
    if (!g_initialized || !g_stats_mutex) {
        return;
    }

    if (xSemaphoreTake(g_stats_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        memset(&g_stats, 0, sizeof(g_stats));
        xSemaphoreGive(g_stats_mutex);
    }
}

bool error_is_in_error_state(void) {
    // Simplified error state check - could be enhanced with more sophisticated logic
    return (g_stats.total_errors > 10 && g_stats.last_error_timestamp > 0 &&
            (get_timestamp_ms() - g_stats.last_error_timestamp) < 5000); // 5 seconds
}

void error_clear_state(void) {
    error_reset_stats();
    ESP_LOGI(TAG, "Error state cleared");
}

bool retry_with_backoff(bool (*func)(void*), void* context,
                       const retry_config_t* config, error_context_t* error_context) {
    if (!func || !config) {
        return false;
    }

    uint32_t attempt = 0;
    bool success = false;

    while (attempt < config->max_attempts && !success) {
        success = func(context);

        if (!success && attempt < (config->max_attempts - 1)) {
            uint32_t delay_ms = calculate_backoff_delay(attempt, config->base_delay_ms,
                                                       config->max_delay_ms,
                                                       config->backoff_multiplier,
                                                       config->jitter_enabled);

            ESP_LOGD(TAG, "Retry attempt %d failed, waiting %d ms", attempt + 1, delay_ms);

            if (delay_ms > 0) {
                vTaskDelay(pdMS_TO_TICKS(delay_ms));
            }
        }

        attempt++;
    }

    if (!success && error_context) {
        // Create error context for the failed operation
        *error_context = error_report(ERROR_CATEGORY_SYSTEM, ERROR_TIMEOUT,
                                    "Operation failed after retries",
                                    __FILE__, __LINE__, __func__, NULL, 0);
    }

    return success;
}

uint32_t calculate_backoff_delay(uint32_t attempt, uint32_t base_delay,
                                uint32_t max_delay, float multiplier, bool jitter_enabled) {
    // Calculate exponential backoff
    uint32_t delay = base_delay;
    for (uint32_t i = 0; i < attempt; i++) {
        delay = (uint32_t)(delay * multiplier);
        if (delay > max_delay) {
            delay = max_delay;
            break;
        }
    }

    // Add jitter if enabled
    if (jitter_enabled && delay < max_delay) {
        uint32_t jitter_max = delay / 4; // Up to 25% jitter
        delay = add_jitter(delay, jitter_max);
    }

    return delay;
}