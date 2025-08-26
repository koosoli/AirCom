/**
 * @file memory_tracker.cpp
 * @brief Memory leak detection and tracking system implementation
 *
 * This file implements a lightweight memory tracking system for detecting
 * potential memory leaks and monitoring heap usage.
 *
 * @author AirCom Development Team
 * @version 1.0.0
 * @date 2024
 */

#include "memory_tracker.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <time.h>
#include <inttypes.h>

static const char* TAG = "MEMORY_TRACKER";

// Global memory tracking state
static bool g_memory_tracking_enabled = false;
static memory_allocation_t g_allocations[MEMORY_TRACKER_MAX_ALLOCATIONS];
static uint32_t g_allocation_count = 0;
static memory_stats_t g_memory_stats = {0};
static TaskHandle_t g_monitoring_task = NULL;

// Internal helper functions
static uint32_t get_current_timestamp(void) {
    return (uint32_t)time(NULL);
}

static uint32_t get_current_thread_id(void) {
    return (uint32_t)xTaskGetCurrentTaskHandle();
}

static int find_allocation_index(void* ptr) {
    for (uint32_t i = 0; i < g_allocation_count; i++) {
        if (g_allocations[i].address == ptr && !g_allocations[i].is_freed) {
            return i;
        }
    }
    return -1;
}

static int find_free_slot(void) {
    // First, try to find a completely free slot
    for (uint32_t i = 0; i < g_allocation_count; i++) {
        if (g_allocations[i].address == NULL) {
            return i;
        }
    }

    // If no free slot found and we have space, use the next slot
    if (g_allocation_count < MEMORY_TRACKER_MAX_ALLOCATIONS) {
        return g_allocation_count++;
    }

    // If we're at capacity, find the oldest freed slot to reuse
    uint32_t oldest_time = UINT32_MAX;
    int oldest_index = -1;

    for (uint32_t i = 0; i < MEMORY_TRACKER_MAX_ALLOCATIONS; i++) {
        if (g_allocations[i].is_freed &&
            g_allocations[i].timestamp < oldest_time) {
            oldest_time = g_allocations[i].timestamp;
            oldest_index = i;
        }
    }

    return oldest_index;
}

// Public API implementation
bool memory_tracker_init(void) {
    if (g_memory_tracking_enabled) {
        ESP_LOGW(TAG, "Memory tracker already initialized");
        return true;
    }

    memset(g_allocations, 0, sizeof(g_allocations));
    memset(&g_memory_stats, 0, sizeof(g_memory_stats));
    g_allocation_count = 0;

    g_memory_stats.last_cleanup_timestamp = get_current_timestamp();
    g_memory_tracking_enabled = true;

    ESP_LOGI(TAG, "Memory tracker initialized");
    return true;
}

void memory_tracker_deinit(void) {
    if (!g_memory_tracking_enabled) {
        return;
    }

    memory_tracker_stop_monitoring();

    // Detect any remaining leaks
    uint32_t leaks = memory_tracker_detect_leaks();
    if (leaks > 0) {
        ESP_LOGW(TAG, "Memory tracker detected %u potential leaks during shutdown", leaks);
    }

    g_memory_tracking_enabled = false;
    ESP_LOGI(TAG, "Memory tracker deinitialized");
}

void memory_tracker_track_allocation(void* ptr, size_t size, const char* file, int line) {
    if (!g_memory_tracking_enabled || !ptr) {
        return;
    }

    int slot = find_free_slot();
    if (slot < 0) {
        ESP_LOGW(TAG, "Memory tracker allocation table full, cannot track allocation");
        return;
    }

    memory_allocation_t* alloc = &g_allocations[slot];

    alloc->address = ptr;
    alloc->size = size;
    alloc->file = file;
    alloc->line = line;
    alloc->timestamp = get_current_timestamp();
    alloc->thread_id = get_current_thread_id();
    alloc->is_freed = false;

#ifdef MEMORY_TRACKER_ENABLE_CALLSTACK
    // Simple callstack capture (simplified for ESP32)
    snprintf(alloc->callstack, sizeof(alloc->callstack),
             "File: %s, Line: %d, Thread: %08X",
             file, line, (unsigned int)alloc->thread_id);
#endif

    // Update statistics
    g_memory_stats.total_allocations++;
    g_memory_stats.current_allocations++;
    g_memory_stats.current_memory_usage += size;

    if (g_memory_stats.current_memory_usage > g_memory_stats.peak_memory_usage) {
        g_memory_stats.peak_memory_usage = g_memory_stats.current_memory_usage;
    }

    // Check usage limits
    int usage_level = memory_tracker_check_usage_limits(80, 95);
    if (usage_level > 0) {
        ESP_LOGW(TAG, "Memory usage at %s level (%zu bytes)",
                usage_level == 1 ? "warning" : "critical",
                g_memory_stats.current_memory_usage);
    }
}

void memory_tracker_track_deallocation(void* ptr, const char* file, int line) {
    if (!g_memory_tracking_enabled || !ptr) {
        return;
    }

    int index = find_allocation_index(ptr);
    if (index < 0) {
        ESP_LOGW(TAG, "Attempting to free untracked memory at %p (file: %s, line: %d)",
                ptr, file, line);
        return;
    }

    memory_allocation_t* alloc = &g_allocations[index];

    // Update statistics
    g_memory_stats.total_deallocations++;
    g_memory_stats.current_allocations--;
    g_memory_stats.current_memory_usage -= alloc->size;

    // Mark as freed
    alloc->is_freed = true;
}

bool memory_tracker_get_stats(memory_stats_t* stats) {
    if (!stats) return false;

    memcpy(stats, &g_memory_stats, sizeof(memory_stats_t));
    return true;
}

uint32_t memory_tracker_detect_leaks(void) {
    if (!g_memory_tracking_enabled) {
        return 0;
    }

    uint32_t leak_count = 0;

    for (uint32_t i = 0; i < MEMORY_TRACKER_MAX_ALLOCATIONS; i++) {
        if (g_allocations[i].address != NULL && !g_allocations[i].is_freed) {
            // Check if this is a recent allocation (not a leak)
            uint32_t age = get_current_timestamp() - g_allocations[i].timestamp;
            if (age > 300) { // 5 minutes threshold
                leak_count++;
                ESP_LOGW(TAG, "Potential memory leak: %zu bytes at %p (%s:%d), age: %u seconds",
                        g_allocations[i].size,
                        g_allocations[i].address,
                        g_allocations[i].file,
                        g_allocations[i].line,
                        age);
            }
        }
    }

    g_memory_stats.memory_leaks = leak_count;
    return leak_count;
}

bool memory_tracker_generate_report(char* report_buffer, size_t buffer_size) {
    if (!report_buffer || buffer_size == 0) {
        return false;
    }

    memory_stats_t stats;
    if (!memory_tracker_get_stats(&stats)) {
        return false;
    }

    uint32_t leaks = memory_tracker_detect_leaks();

    snprintf(report_buffer, buffer_size,
             "Memory Report:\n"
             "  Total Allocations: %" PRIu32 "\n"
             "  Total Deallocations: %" PRIu32 "\n"
             "  Current Allocations: %" PRIu32 "\n"
             "  Peak Memory Usage: %zu bytes\n"
             "  Current Memory Usage: %zu bytes\n"
             "  Memory Leaks: %" PRIu32 "\n"
             "  Allocation Failures: %" PRIu32 "\n"
             "  Fragmentation Events: %" PRIu32 "\n",
             stats.total_allocations,
             stats.total_deallocations,
             stats.current_allocations,
             stats.peak_memory_usage,
             stats.current_memory_usage,
             leaks,
             stats.allocation_failures,
             stats.fragmentation_count);

    return true;
}

void memory_tracker_cleanup_old_records(uint32_t max_age_seconds) {
    if (!g_memory_tracking_enabled) {
        return;
    }

    uint32_t current_time = get_current_timestamp();
    uint32_t cleaned = 0;

    for (uint32_t i = 0; i < MEMORY_TRACKER_MAX_ALLOCATIONS; i++) {
        if (g_allocations[i].address != NULL &&
            g_allocations[i].is_freed &&
            (current_time - g_allocations[i].timestamp) > max_age_seconds) {

            memset(&g_allocations[i], 0, sizeof(memory_allocation_t));
            cleaned++;
        }
    }

    if (cleaned > 0) {
        ESP_LOGI(TAG, "Cleaned up %u old memory records", cleaned);
    }

    g_memory_stats.last_cleanup_timestamp = current_time;
}

int memory_tracker_check_usage_limits(uint8_t warning_threshold, uint8_t critical_threshold) {
    // Get total heap size (simplified for ESP32)
    size_t total_heap = heap_caps_get_total_size(MALLOC_CAP_8BIT);

    if (total_heap == 0) {
        return 0; // Cannot determine usage
    }

    uint8_t usage_percentage = (g_memory_stats.current_memory_usage * 100) / total_heap;

    if (usage_percentage >= critical_threshold) {
        return 2; // Critical
    } else if (usage_percentage >= warning_threshold) {
        return 1; // Warning
    }

    return 0; // Normal
}

const memory_allocation_t* memory_tracker_get_allocation_info(void* ptr) {
    if (!g_memory_tracking_enabled || !ptr) {
        return NULL;
    }

    int index = find_allocation_index(ptr);
    if (index >= 0) {
        return &g_allocations[index];
    }

    return NULL;
}

void memory_tracker_set_enabled(bool enable) {
    if (enable && !g_memory_tracking_enabled) {
        memory_tracker_init();
    } else if (!enable && g_memory_tracking_enabled) {
        memory_tracker_deinit();
    }
}

bool memory_tracker_is_enabled(void) {
    return g_memory_tracking_enabled;
}

// Debug and monitoring functions
void memory_tracker_print_stats(void) {
    if (!g_memory_tracking_enabled) {
        ESP_LOGI(TAG, "Memory tracking not enabled");
        return;
    }

    memory_stats_t stats;
    if (memory_tracker_get_stats(&stats)) {
        ESP_LOGI(TAG, "=== Memory Statistics ===");
        ESP_LOGI(TAG, "Total Allocations: %u", stats.total_allocations);
        ESP_LOGI(TAG, "Total Deallocations: %u", stats.total_deallocations);
        ESP_LOGI(TAG, "Current Allocations: %u", stats.current_allocations);
        ESP_LOGI(TAG, "Peak Memory Usage: %zu bytes", stats.peak_memory_usage);
        ESP_LOGI(TAG, "Current Memory Usage: %zu bytes", stats.current_memory_usage);
        ESP_LOGI(TAG, "Memory Leaks: %u", stats.memory_leaks);
        ESP_LOGI(TAG, "==========================");
    }
}

void memory_tracker_print_leaks(void) {
    if (!g_memory_tracking_enabled) {
        return;
    }

    uint32_t leaks = memory_tracker_detect_leaks();
    if (leaks == 0) {
        ESP_LOGI(TAG, "No memory leaks detected");
    } else {
        ESP_LOGW(TAG, "Detected %u potential memory leaks", leaks);
    }
}

// Monitoring task function
static void memory_monitoring_task(void* pvParameters) {
    uint32_t interval_seconds = (uint32_t)pvParameters;

    ESP_LOGI(TAG, "Memory monitoring task started (interval: %u seconds)", interval_seconds);

    while (g_memory_tracking_enabled) {
        // Wait for the specified interval
        vTaskDelay(pdMS_TO_TICKS(interval_seconds * 1000));

        if (!g_memory_tracking_enabled) {
            break;
        }

        // Perform monitoring
        memory_stats_t stats;
        if (memory_tracker_get_stats(&stats)) {
            // Check for concerning patterns
            if (stats.current_allocations > 100) {
                ESP_LOGW(TAG, "High number of active allocations: %u", stats.current_allocations);
            }

            if (stats.memory_leaks > 0) {
                ESP_LOGW(TAG, "Memory leaks detected: %u", stats.memory_leaks);
            }
        }

        // Periodic cleanup of old records (keep only last 24 hours)
        memory_tracker_cleanup_old_records(24 * 60 * 60);
    }

    ESP_LOGI(TAG, "Memory monitoring task stopped");
    vTaskDelete(NULL);
}

bool memory_tracker_start_monitoring(uint32_t interval_seconds) {
    if (g_monitoring_task != NULL) {
        ESP_LOGW(TAG, "Memory monitoring already running");
        return false;
    }

    if (xTaskCreate(memory_monitoring_task,
                   "MemoryMonitor",
                   2048,
                   (void*)interval_seconds,
                   1,
                   &g_monitoring_task) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create memory monitoring task");
        return false;
    }

    return true;
}

void memory_tracker_stop_monitoring(void) {
    if (g_monitoring_task != NULL) {
        vTaskDelete(g_monitoring_task);
        g_monitoring_task = NULL;
    }
}