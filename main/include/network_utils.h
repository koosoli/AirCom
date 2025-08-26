#ifndef NETWORK_UTILS_H
#define NETWORK_UTILS_H

// ============================================================================
// NETWORK UTILITIES - PRODUCTION READY IMPLEMENTATION
//
// This module provides network utility functions for the AirCom system.
// Features:
// - TCP message sending with error handling and retry logic
// - UDP multicast broadcasting
// - Network initialization and configuration
// - Socket management with proper cleanup
// - Network diagnostics and monitoring
//
// Usage:
// 1. Call network_utils_init() at system startup
// 2. Use send_tcp_message() for reliable message delivery
// 3. Use broadcast_discovery_packet() for service discovery
// 4. Monitor network health with network_get_status()
// ============================================================================

#include <stdint.h>
#include <stdbool.h>
#include <vector>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// NETWORK CONFIGURATION
// ============================================================================

// Network timeout configurations
#define NETWORK_CONNECT_TIMEOUT_MS 5000
#define NETWORK_SEND_TIMEOUT_MS 3000
#define NETWORK_RECEIVE_TIMEOUT_MS 5000
#define NETWORK_RETRY_DELAY_MS 1000

// Buffer sizes
#define NETWORK_MAX_MESSAGE_SIZE 4096
#define NETWORK_RECV_BUFFER_SIZE 8192

// Socket configuration
#define NETWORK_MAX_BACKLOG 5
#define NETWORK_LINGER_TIMEOUT_S 1

// ============================================================================
// NETWORK STATUS AND STATISTICS
// ============================================================================

typedef enum {
    NETWORK_STATUS_DISCONNECTED = 0,
    NETWORK_STATUS_CONNECTING,
    NETWORK_STATUS_CONNECTED,
    NETWORK_STATUS_ERROR,
    NETWORK_STATUS_RECONNECTING
} network_status_t;

typedef struct {
    uint32_t total_messages_sent;
    uint32_t total_messages_received;
    uint32_t total_bytes_sent;
    uint32_t total_bytes_received;
    uint32_t connection_attempts;
    uint32_t successful_connections;
    uint32_t failed_connections;
    uint32_t timeout_errors;
    uint32_t network_errors;
    uint32_t last_activity_timestamp;
    network_status_t current_status;
} network_stats_t;

// ============================================================================
// NETWORK UTILITIES API
// ============================================================================

/**
 * @brief Initialize network utilities system
 *
 * @return true on success, false on failure
 */
bool network_utils_init(void);

/**
 * @brief Deinitialize network utilities system
 */
void network_utils_deinit(void);

/**
 * @brief Send a TCP message with error handling and retry logic
 *
 * @param host_ip Target IP address
 * @param payload Message payload
 * @param max_retries Maximum number of retry attempts
 * @return true on success, false on failure
 */
bool send_tcp_message(const char* host_ip, const std::vector<uint8_t>& payload, int max_retries);

/**
 * @brief Send a TCP message with default retry settings
 *
 * @param host_ip Target IP address
 * @param payload Message payload
 * @return true on success, false on failure
 */
bool send_tcp_message_default(const char* host_ip, const std::vector<uint8_t>& payload);

/**
 * @brief Broadcast UDP discovery packet
 *
 * @param payload Discovery packet payload
 * @param payload_size Size of payload
 * @param port Target port
 * @return true on success, false on failure
 */
bool broadcast_udp_packet(const uint8_t* payload, size_t payload_size, uint16_t port);

/**
 * @brief Get network interface IP address
 *
 * @param ip_buffer Buffer to store IP address string
 * @param buffer_size Size of buffer
 * @return true on success, false on failure
 */
bool get_local_ip(char* ip_buffer, size_t buffer_size);

/**
 * @brief Check if network interface is available
 *
 * @return true if network is available, false otherwise
 */
bool is_network_available(void);

/**
 * @brief Get network statistics
 *
 * @param stats Pointer to store statistics
 * @return true on success, false on failure
 */
bool network_get_stats(network_stats_t* stats);

/**
 * @brief Reset network statistics
 */
void network_reset_stats(void);

/**
 * @brief Get current network status
 *
 * @return Current network status
 */
network_status_t network_get_status(void);

/**
 * @brief Set network status
 *
 * @param status New network status
 */
void network_set_status(network_status_t status);

/**
 * @brief Validate IP address format
 *
 * @param ip IP address string
 * @return true if valid, false otherwise
 */
bool validate_ip_address(const char* ip);

/**
 * @brief Get hostname from IP address (DNS lookup)
 *
 * @param ip IP address string
 * @param hostname_buffer Buffer to store hostname
 * @param buffer_size Size of buffer
 * @return true on success, false on failure
 */
bool resolve_hostname(const char* ip, char* hostname_buffer, size_t buffer_size);

/**
 * @brief Test network connectivity to a specific host
 *
 * @param host_ip Target IP address
 * @param timeout_ms Timeout in milliseconds
 * @return true if reachable, false otherwise
 */
bool test_connectivity(const char* host_ip, uint32_t timeout_ms);

/**
 * @brief Get network interface information
 *
 * @param interface_name Buffer to store interface name
 * @param name_size Size of name buffer
 * @param mac_address Buffer to store MAC address (6 bytes)
 * @return true on success, false on failure
 */
bool get_network_interface_info(char* interface_name, size_t name_size, uint8_t* mac_address);

/**
 * @brief Enable/disable network debugging
 *
 * @param enable true to enable debugging, false to disable
 */
void network_set_debug(bool enable);

int receive_udp_packet(uint8_t* rx_buffer, size_t rx_buffer_size, char* source_ip, size_t source_ip_size);

#ifdef __cplusplus
}
#endif

#endif // NETWORK_UTILS_H