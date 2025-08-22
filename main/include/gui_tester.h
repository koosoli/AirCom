/**
 * @file gui_tester.h
 * @brief GUI Testing and Troubleshooting System
 *
 * This file provides a comprehensive testing and troubleshooting system
 * for the AirCom GUI and system functionality. It allows simulation,
 * debugging, and validation of UI components and system behavior.
 *
 * @author AirCom Development Team
 * @version 1.0.0
 * @date 2024
 */

#ifndef GUI_TESTER_H
#define GUI_TESTER_H

#include <stdint.h>
#include <stdbool.h>
#include <string>
#include <vector>
#include <map>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// GUI TEST MODES
// ============================================================================

typedef enum {
    GUI_TEST_MODE_NONE = 0,        // Normal operation
    GUI_TEST_MODE_SIMULATION,      // Simulate UI without hardware
    GUI_TEST_MODE_DEBUG,           // Debug mode with extra logging
    GUI_TEST_MODE_STRESS_TEST,     // Stress testing mode
    GUI_TEST_MODE_DIAGNOSTIC       // Diagnostic mode
} gui_test_mode_t;

// ============================================================================
// UI SIMULATION DATA
// ============================================================================

typedef struct {
    std::string screen_name;
    uint32_t screen_id;
    std::vector<std::string> elements;
    bool has_buttons;
    bool has_display;
    uint32_t update_rate_hz;
} ui_screen_info_t;

typedef struct {
    std::string button_name;
    bool pressed;
    uint32_t press_count;
    uint32_t last_press_time;
} ui_button_state_t;

typedef struct {
    std::string element_name;
    std::string element_type;  // "text", "icon", "progress", "graph"
    int x_pos;
    int y_pos;
    int width;
    int height;
    std::string content;
    bool visible;
    uint32_t update_time;
} ui_element_state_t;

typedef struct {
    std::string display_content;
    uint32_t brightness;
    bool backlight_on;
    uint32_t refresh_count;
    uint32_t error_count;
} display_state_t;

// ============================================================================
// SYSTEM STATE SIMULATION
// ============================================================================

typedef struct {
    bool has_gps_lock;
    float latitude;
    float longitude;
    float altitude;
    uint32_t satellites;
    uint32_t contact_count;
    std::vector<std::string> contact_list;
    uint32_t message_count;
    uint32_t audio_level;
    bool ptt_pressed;
    uint32_t battery_level;
    std::string current_screen;
} system_state_t;

// ============================================================================
// TEST CONFIGURATION
// ============================================================================

typedef struct {
    gui_test_mode_t mode;
    bool simulate_hardware;
    bool log_ui_events;
    bool log_system_events;
    bool simulate_errors;
    uint32_t error_injection_rate;  // Percentage (0-100)
    uint32_t test_duration_ms;
    std::string output_file;
    bool enable_screenshots;
    uint32_t screenshot_interval_ms;
} gui_test_config_t;

// ============================================================================
// DIAGNOSTIC INFORMATION
// ============================================================================

typedef struct {
    uint32_t ui_updates_total;
    uint32_t ui_errors_total;
    uint32_t button_events_total;
    uint32_t screen_transitions_total;
    uint32_t memory_usage_peak;
    uint32_t memory_usage_current;
    uint32_t cpu_usage_peak;
    float avg_response_time_ms;
    std::vector<std::string> error_log;
    std::map<std::string, uint32_t> event_counts;
} diagnostic_info_t;

// ============================================================================
// GUI TESTER API
// ============================================================================

/**
 * @brief Initialize the GUI testing system
 *
 * @param config Test configuration
 * @return true on success, false on failure
 */
bool gui_tester_init(const gui_test_config_t* config);

/**
 * @brief Deinitialize the GUI testing system
 */
void gui_tester_deinit(void);

/**
 * @brief Start GUI testing session
 *
 * @return true on success, false on failure
 */
bool gui_tester_start(void);

/**
 * @brief Stop GUI testing session
 */
void gui_tester_stop(void);

/**
 * @brief Simulate button press
 *
 * @param button_name Name of button to press
 * @param duration_ms Press duration in milliseconds
 * @return true on success, false on failure
 */
bool gui_tester_simulate_button_press(const char* button_name, uint32_t duration_ms);

/**
 * @brief Simulate system event
 *
 * @param event_type Type of event ("gps_lock", "contact_found", "message_received", etc.)
 * @param event_data Event-specific data
 * @return true on success, false on failure
 */
bool gui_tester_simulate_system_event(const char* event_type, const char* event_data);

/**
 * @brief Inject error for testing
 *
 * @param error_type Type of error to inject
 * @param error_data Error-specific data
 * @return true on success, false on failure
 */
bool gui_tester_inject_error(const char* error_type, const char* error_data);

/**
 * @brief Get current UI state
 *
 * @param screen_info Output screen information
 * @param button_states Output button states
 * @param display_state Output display state
 * @return true on success, false on failure
 */
bool gui_tester_get_ui_state(ui_screen_info_t* screen_info,
                           std::vector<ui_button_state_t>* button_states,
                           display_state_t* display_state);

/**
 * @brief Get current system state
 *
 * @param system_state Output system state
 * @return true on success, false on failure
 */
bool gui_tester_get_system_state(system_state_t* system_state);

/**
 * @brief Get diagnostic information
 *
 * @param diagnostics Output diagnostic information
 * @return true on success, false on failure
 */
bool gui_tester_get_diagnostics(diagnostic_info_t* diagnostics);

/**
 * @brief Generate test report
 *
 * @param report_buffer Buffer to store report
 * @param buffer_size Size of buffer
 * @return true on success, false on failure
 */
bool gui_tester_generate_report(char* report_buffer, size_t buffer_size);

/**
 * @brief Take UI screenshot (simulation)
 *
 * @param filename Output filename
 * @return true on success, false on failure
 */
bool gui_tester_take_screenshot(const char* filename);

// ============================================================================
// TROUBLESHOOTING FUNCTIONS
// ============================================================================

/**
 * @brief Run UI responsiveness test
 *
 * @return Test results as string
 */
std::string gui_tester_run_responsiveness_test(void);

/**
 * @brief Run memory usage analysis
 *
 * @return Analysis results as string
 */
std::string gui_tester_run_memory_analysis(void);

/**
 * @brief Run button functionality test
 *
 * @return Test results as string
 */
std::string gui_tester_run_button_test(void);

/**
 * @brief Run display rendering test
 *
 * @return Test results as string
 */
std::string gui_tester_run_display_test(void);

/**
 * @brief Check for common UI issues
 *
 * @return Issues found as string
 */
std::string gui_tester_check_common_issues(void);

// ============================================================================
// DEBUGGING FUNCTIONS
// ============================================================================

/**
 * @brief Enable debug logging for specific component
 *
 * @param component Component name ("UI", "NETWORK", "AUDIO", etc.)
 * @param enable true to enable, false to disable
 */
void gui_tester_enable_component_debug(const char* component, bool enable);

/**
 * @brief Set UI update rate for testing
 *
 * @param rate_hz Update rate in Hz
 */
void gui_tester_set_ui_update_rate(uint32_t rate_hz);

/**
 * @brief Enable UI event tracing
 *
 * @param enable true to enable, false to disable
 */
void gui_tester_enable_event_tracing(bool enable);

/**
 * @brief Get UI event trace
 *
 * @return Event trace as string
 */
std::string gui_tester_get_event_trace(void);

// ============================================================================
// CONSOLE COMMANDS FOR TESTING
// ============================================================================

/**
 * @brief Process console command for testing
 *
 * @param command Command string
 * @return Response string
 */
std::string gui_tester_process_console_command(const char* command);

/**
 * @brief Get help text for console commands
 *
 * @return Help text
 */
std::string gui_tester_get_console_help(void);

// Example console commands:
// "help" - Show help
// "status" - Show current status
// "simulate button_press ptt 100" - Simulate PTT button press for 100ms
// "simulate gps_lock" - Simulate GPS lock
// "inject error memory_full" - Inject memory error
// "screenshot ui_state.png" - Take UI screenshot
// "test responsiveness" - Run responsiveness test
// "test memory" - Run memory analysis
// "diagnostics" - Show diagnostic information

// ============================================================================
// SYSTEM MONITORING
// ============================================================================

/**
 * @brief Get system performance metrics
 *
 * @param cpu_usage Output CPU usage percentage
 * @param memory_usage Output memory usage bytes
 * @param ui_response_time Output UI response time in ms
 * @return true on success, false on failure
 */
bool gui_tester_get_performance_metrics(uint32_t* cpu_usage,
                                      uint32_t* memory_usage,
                                      uint32_t* ui_response_time);

/**
 * @brief Monitor system for issues
 *
 * @param monitoring_duration_ms Duration to monitor in milliseconds
 * @return Issues found as string
 */
std::string gui_tester_monitor_system(uint32_t monitoring_duration_ms);

/**
 * @brief Check system health
 *
 * @return Health status as string
 */
std::string gui_tester_check_system_health(void);

// ============================================================================
// TEST AUTOMATION
// ============================================================================

/**
 * @brief Run automated test suite
 *
 * @param test_suite_name Name of test suite to run
 * @return Test results as string
 */
std::string gui_tester_run_automated_test_suite(const char* test_suite_name);

/**
 * @brief Create custom test scenario
 *
 * @param scenario_name Name of scenario
 * @param scenario_steps Vector of steps to execute
 * @return true on success, false on failure
 */
bool gui_tester_create_test_scenario(const char* scenario_name,
                                   const std::vector<std::string>& scenario_steps);

/**
 * @brief Run custom test scenario
 *
 * @param scenario_name Name of scenario to run
 * @return Test results as string
 */
std::string gui_tester_run_test_scenario(const char* scenario_name);

// ============================================================================
// VISUALIZATION AND DEBUGGING TOOLS
// ============================================================================

/**
 * @brief Generate UI layout visualization
 *
 * @param output_file Output file for visualization
 * @return true on success, false on failure
 */
bool gui_tester_generate_ui_layout(const char* output_file);

/**
 * @brief Create system state dump
 *
 * @param output_file Output file for state dump
 * @return true on success, false on failure
 */
bool gui_tester_create_state_dump(const char* output_file);

/**
 * @brief Visualize system interactions
 *
 * @param output_file Output file for interaction diagram
 * @return true on success, false on failure
 */
bool gui_tester_visualize_interactions(const char* output_file);

#ifdef __cplusplus
}
#endif

#endif // GUI_TESTER_H