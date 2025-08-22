/**
 * @file memory_tracker.h
 * @brief Memory leak detection and tracking system
 *
 * This file provides a lightweight memory tracking system for detecting
 * potential memory leaks and monitoring heap usage in the AirCom system.
 *
 * @author AirCom Development Team
 * @version 1.0.0
 * @date 2024
 */

#ifndef MEMORY_TRACKER_H
#define MEMORY_TRACKER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// MEMORY TRACKING CONFIGURATION
// ============================================================================

#define MEMORY_TRACKER_MAX_ALLOCATIONS 1000
#define MEMORY_TRACKER_CALLSTACK_DEPTH 10
#define MEMORY_TRACKER_ENABLE_CALLSTACK 1

// ============================================================================
// MEMORY ALLOCATION ENTRY
// ============================================================================

typedef struct {
    void* address;                    // Allocated memory address
    size_t size;                      // Size of allocation
    const char* file;                 // Source file
    int line;                         // Line number
    uint32_t timestamp;               // Allocation timestamp
    uint32_t thread_id;               // Allocating thread ID
    bool is_freed;                    // Whether this allocation has been freed
    char callstack[MEMORY_TRACKER_CALLSTACK_DEPTH * 20]; // Callstack info
} memory_allocation_t;

// ============================================================================
// MEMORY STATISTICS
// ============================================================================

typedef struct {
    uint32_t total_allocations;       // Total number of allocations
    uint32_t total_deallocations;     // Total number of deallocations
    uint32_t current_allocations;     // Currently active allocations
    size_t peak_memory_usage;         // Peak memory usage
    size_t current_memory_usage;      // Current memory usage
    uint32_t memory_leaks;            // Number of detected memory leaks
    uint32_t allocation_failures;     // Number of failed allocations
    uint32_t fragmentation_count;     // Memory fragmentation events
    uint32_t last_cleanup_timestamp;  // Last cleanup timestamp
} memory_stats_t;

// ============================================================================
// MEMORY TRACKER API
// ============================================================================

/**
 * @brief Initialize the memory tracking system
 *
 * @return true on success, false on failure
 */
bool memory_tracker_init(void);

/**
 * @brief Deinitialize the memory tracking system
 */
void memory_tracker_deinit(void);

/**
 * @brief Track a memory allocation
 *
 * @param ptr Allocated memory address
 * @param size Size of allocation
 * @param file Source file name
 * @param line Line number
 */
void memory_tracker_track_allocation(void* ptr, size_t size, const char* file, int line);

/**
 * @brief Track a memory deallocation
 *
 * @param ptr Memory address being freed
 * @param file Source file name
 * @param line Line number
 */
void memory_tracker_track_deallocation(void* ptr, const char* file, int line);

/**
 * @brief Get current memory statistics
 *
 * @param stats Pointer to store statistics
 * @return true on success, false on failure
 */
bool memory_tracker_get_stats(memory_stats_t* stats);

/**
 * @brief Detect memory leaks and generate report
 *
 * @return Number of detected memory leaks
 */
uint32_t memory_tracker_detect_leaks(void);

/**
 * @brief Generate memory usage report
 *
 * @param report_buffer Buffer to store report
 * @param buffer_size Size of buffer
 * @return true on success, false on failure
 */
bool memory_tracker_generate_report(char* report_buffer, size_t buffer_size);

/**
 * @brief Cleanup old allocation records
 *
 * @param max_age_seconds Maximum age of records to keep
 */
void memory_tracker_cleanup_old_records(uint32_t max_age_seconds);

/**
 * @brief Check if memory usage is within safe limits
 *
 * @param warning_threshold Warning threshold (0-100)
 * @param critical_threshold Critical threshold (0-100)
 * @return 0=normal, 1=warning, 2=critical
 */
int memory_tracker_check_usage_limits(uint8_t warning_threshold, uint8_t critical_threshold);

/**
 * @brief Get allocation information by address
 *
 * @param ptr Memory address
 * @return Allocation entry or NULL if not found
 */
const memory_allocation_t* memory_tracker_get_allocation_info(void* ptr);

/**
 * @brief Enable/disable memory tracking
 *
 * @param enable true to enable, false to disable
 */
void memory_tracker_set_enabled(bool enable);

/**
 * @brief Check if memory tracking is enabled
 *
 * @return true if enabled, false otherwise
 */
bool memory_tracker_is_enabled(void);

// ============================================================================
// MEMORY TRACKING MACROS
// ============================================================================

#ifdef CONFIG_MEMORY_TRACKING_ENABLE

#define MALLOC_TRACKED(size) \
    ({ \
        void* ptr = malloc(size); \
        if (ptr) { \
            memory_tracker_track_allocation(ptr, size, __FILE__, __LINE__); \
        } \
        ptr; \
    })

#define FREE_TRACKED(ptr) \
    do { \
        if (ptr) { \
            memory_tracker_track_deallocation(ptr, __FILE__, __LINE__); \
            free(ptr); \
        } \
    } while (0)

#define CALLOC_TRACKED(nmemb, size) \
    ({ \
        void* ptr = calloc(nmemb, size); \
        if (ptr) { \
            memory_tracker_track_allocation(ptr, nmemb * size, __FILE__, __LINE__); \
        } \
        ptr; \
    })

#define REALLOC_TRACKED(ptr, size) \
    ({ \
        void* new_ptr = realloc(ptr, size); \
        if (new_ptr != ptr) { \
            if (ptr) { \
                memory_tracker_track_deallocation(ptr, __FILE__, __LINE__); \
            } \
            if (new_ptr) { \
                memory_tracker_track_allocation(new_ptr, size, __FILE__, __LINE__); \
            } \
        } \
        new_ptr; \
    })

#else // CONFIG_MEMORY_TRACKING_ENABLE not defined

#define MALLOC_TRACKED(size) malloc(size)
#define FREE_TRACKED(ptr) free(ptr)
#define CALLOC_TRACKED(nmemb, size) calloc(nmemb, size)
#define REALLOC_TRACKED(ptr, size) realloc(ptr, size)

#endif // CONFIG_MEMORY_TRACKING_ENABLE

// ============================================================================
// DEBUG AND MONITORING FUNCTIONS
// ============================================================================

/**
 * @brief Print current memory statistics to console
 */
void memory_tracker_print_stats(void);

/**
 * @brief Print detailed memory leak report to console
 */
void memory_tracker_print_leaks(void);

/**
 * @brief Start periodic memory monitoring task
 *
 * @param interval_seconds Monitoring interval
 * @return true on success, false on failure
 */
bool memory_tracker_start_monitoring(uint32_t interval_seconds);

/**
 * @brief Stop periodic memory monitoring task
 */
void memory_tracker_stop_monitoring(void);

#ifdef __cplusplus
}
#endif

#endif // MEMORY_TRACKER_H