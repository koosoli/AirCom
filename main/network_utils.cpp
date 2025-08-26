/**
 * @file network_utils.c
 * @brief Network utilities implementation for AirCom system
 *
 * This file provides network utility functions for the AirCom tactical communication system.
 * Implements TCP messaging, UDP broadcasting, network diagnostics, and socket management
 * with proper error handling and retry logic.
 *
 * @author AirCom Development Team
 * @version 1.0.0
 * @date 2024
 */

#include "include/network_utils.h"
#include "include/error_handling.h"

// ESP-IDF includes
#include "esp_system.h"
#include "esp_log.h"
#include "esp_err.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "include/logging_system.h"
#include "include/config.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

// Global network statistics
static network_stats_t g_network_stats = {0};
static network_status_t g_network_status = NETWORK_STATUS_DISCONNECTED;
static bool g_debug_enabled = false;

// Internal helper functions
static int create_tcp_socket(void);
static int set_socket_timeout(int sock, int timeout_ms);
static bool is_valid_ip_format(const char* ip);

/**
 * @brief Initialize network utilities system
 */
bool network_utils_init(void) {
    // Reset statistics
    memset(&g_network_stats, 0, sizeof(network_stats_t));
    g_network_status = NETWORK_STATUS_DISCONNECTED;

    // Mark initialization time
    g_network_stats.last_activity_timestamp = (uint32_t)time(NULL);

    LOG_NETWORK_INFO("Network utilities initialized successfully");
    return true;
}

int receive_udp_packet(uint8_t* rx_buffer, size_t rx_buffer_size, char* source_ip, size_t source_ip_size) {
    // Dummy implementation
    return 0;
}

/**
 * @brief Deinitialize network utilities system
 */
void network_utils_deinit(void) {
    g_network_status = NETWORK_STATUS_DISCONNECTED;
    LOG_NETWORK_INFO("Network utilities deinitialized");
}

/**
 * @brief Send a TCP message with error handling and retry logic
 */
bool send_tcp_message(const char* host_ip, const std::vector<uint8_t>& payload, int max_retries) {
    if (!host_ip || payload.empty() || max_retries < 0) {
        LOG_NETWORK_ERROR(ERROR_INVALID_PARAMETER, "Invalid parameters for send_tcp_message");
        g_network_stats.network_errors++;
        return false;
    }

    if (!is_valid_ip_format(host_ip)) {
        LOG_NETWORK_ERROR(ERROR_INVALID_ADDRESS, "Invalid IP address format: %s", host_ip);
        g_network_stats.network_errors++;
        return false;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(TEXT_PORT);

    if (inet_pton(AF_INET, host_ip, &server_addr.sin_addr) <= 0) {
        LOG_NETWORK_ERROR(ERROR_INVALID_ADDRESS, "Invalid IP address: %s", host_ip);
        g_network_stats.network_errors++;
        return false;
    }

    bool success = false;
    int attempt = 0;

    while (attempt <= max_retries && !success) {
        g_network_stats.total_messages_sent++;
        g_network_stats.connection_attempts++;

        int sock = create_tcp_socket();
        if (sock < 0) {
            LOG_NETWORK_ERROR(ERROR_SOCKET_CREATE, "Failed to create TCP socket (attempt %d)", attempt + 1);
            g_network_stats.network_errors++;
            attempt++;
            if (attempt <= max_retries) {
                sleep(NETWORK_RETRY_DELAY_MS / 1000);
            }
            continue;
        }

        // Set socket timeout
        if (!set_socket_timeout(sock, NETWORK_CONNECT_TIMEOUT_MS)) {
            LOG_NETWORK_WARNING("Failed to set socket timeout");
        }

        // Attempt connection
        int connect_result = connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
        if (connect_result < 0) {
            LOG_NETWORK_ERROR(ERROR_SOCKET_CONNECT, "Connection failed (attempt %d): %s", attempt + 1, strerror(errno));
            close(sock);
            g_network_stats.failed_connections++;
            g_network_stats.timeout_errors++;
            attempt++;
            if (attempt <= max_retries) {
                sleep(NETWORK_RETRY_DELAY_MS / 1000);
            }
            continue;
        }

        g_network_stats.successful_connections++;
        g_network_status = NETWORK_STATUS_CONNECTED;
        g_network_stats.last_activity_timestamp = (uint32_t)time(NULL);

        // Send data with timeout
        size_t total_sent = 0;
        const uint8_t* data = payload.data();
        size_t data_size = payload.size();

        while (total_sent < data_size) {
            int sent = send(sock, data + total_sent, data_size - total_sent, 0);
            if (sent < 0) {
                LOG_NETWORK_ERROR(ERROR_SOCKET_SEND, "Send failed: %s", strerror(errno));
                close(sock);
                g_network_stats.network_errors++;
                success = false;
                break;
            }
            total_sent += sent;
            g_network_stats.total_bytes_sent += sent;
        }

        if (total_sent == data_size) {
            success = true;
            if (g_debug_enabled) {
                LOG_NETWORK_DEBUG("Successfully sent %zu bytes to %s", data_size, host_ip);
            }
        }

        close(sock);

        if (!success) {
            attempt++;
            if (attempt <= max_retries) {
                sleep(NETWORK_RETRY_DELAY_MS / 1000);
            }
        }
    }

    if (!success) {
        g_network_status = NETWORK_STATUS_ERROR;
        LOG_NETWORK_ERROR(ERROR_CONNECTION_LOST, "Failed to send message to %s after %d attempts", host_ip, max_retries + 1);
    }

    return success;
}

/**
 * @brief Send a TCP message with default retry settings
 */
bool send_tcp_message_default(const char* host_ip, const std::vector<uint8_t>& payload) {
    return send_tcp_message(host_ip, payload, 3); // Default 3 retries
}

/**
 * @brief Broadcast UDP discovery packet
 */
bool broadcast_udp_packet(const uint8_t* payload, size_t payload_size, uint16_t port) {
    if (!payload || payload_size == 0 || payload_size > NETWORK_MAX_MESSAGE_SIZE) {
        LOG_NETWORK_ERROR(ERROR_INVALID_PARAMETER, "Invalid parameters for broadcast_udp_packet");
        return false;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        LOG_NETWORK_ERROR(ERROR_SOCKET_CREATE, "Failed to create UDP socket: %s", strerror(errno));
        g_network_stats.network_errors++;
        return false;
    }

    // Set broadcast option
    int broadcast_enable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) < 0) {
        LOG_NETWORK_ERROR(ERROR_SOCKET_CREATE, "Failed to set broadcast option: %s", strerror(errno));
        close(sock);
        return false;
    }

    // Set socket timeout
    if (!set_socket_timeout(sock, NETWORK_SEND_TIMEOUT_MS)) {
        LOG_NETWORK_WARNING("Failed to set UDP socket timeout");
    }

    struct sockaddr_in broadcast_addr;
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(port);
    broadcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    size_t total_sent = 0;
    int attempts = 0;
    const int max_attempts = 3;

    while (total_sent < payload_size && attempts < max_attempts) {
        int sent = sendto(sock, payload + total_sent, payload_size - total_sent, 0,
                         (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
        if (sent < 0) {
            LOG_NETWORK_ERROR(ERROR_SOCKET_SEND, "UDP broadcast failed (attempt %d): %s", attempts + 1, strerror(errno));
            attempts++;
            if (attempts < max_attempts) {
                sleep(NETWORK_RETRY_DELAY_MS / 1000);
            }
        } else {
            total_sent += sent;
            g_network_stats.total_bytes_sent += sent;
            if (g_debug_enabled) {
                LOG_NETWORK_DEBUG("UDP broadcast sent %d bytes", sent);
            }
        }
    }

    close(sock);

    if (total_sent == payload_size) {
        g_network_stats.total_messages_sent++;
        g_network_stats.last_activity_timestamp = (uint32_t)time(NULL);
        return true;
    } else {
        g_network_stats.network_errors++;
        return false;
    }
}

/**
 * @brief Get local IP address
 */
bool get_local_ip(char* ip_buffer, size_t buffer_size) {
    if (!ip_buffer || buffer_size == 0) {
        return false;
    }

    struct ifaddrs* interfaces = NULL;
    if (getifaddrs(&interfaces) != 0) {
        LOG_NETWORK_ERROR(ERROR_SYSTEM_OVERLOAD, "Failed to get network interfaces: %s", strerror(errno));
        return false;
    }

    bool found = false;
    for (struct ifaddrs* ifa = interfaces; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;

        if (ifa->ifa_addr->sa_family == AF_INET && !(ifa->ifa_flags & IFF_LOOPBACK)) {
            char ip[INET_ADDRSTRLEN];
            struct sockaddr_in* addr = (struct sockaddr_in*)ifa->ifa_addr;
            if (inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip))) {
                if (strlen(ip) < buffer_size) {
                    strcpy(ip_buffer, ip);
                    found = true;
                    break;
                }
            }
        }
    }

    freeifaddrs(interfaces);
    return found;
}

/**
 * @brief Check if network interface is available
 */
bool is_network_available(void) {
    char ip_buffer[INET_ADDRSTRLEN];
    return get_local_ip(ip_buffer, sizeof(ip_buffer));
}

/**
 * @brief Get network statistics
 */
bool network_get_stats(network_stats_t* stats) {
    if (!stats) return false;
    memcpy(stats, &g_network_stats, sizeof(network_stats_t));
    return true;
}

/**
 * @brief Reset network statistics
 */
void network_reset_stats(void) {
    memset(&g_network_stats, 0, sizeof(network_stats_t));
    g_network_stats.last_activity_timestamp = (uint32_t)time(NULL);
}

/**
 * @brief Get current network status
 */
network_status_t network_get_status(void) {
    return g_network_status;
}

/**
 * @brief Set network status
 */
void network_set_status(network_status_t status) {
    g_network_status = status;
    g_network_stats.last_activity_timestamp = (uint32_t)time(NULL);
}

/**
 * @brief Validate IP address format
 */
bool validate_ip_address(const char* ip) {
    return is_valid_ip_format(ip);
}

/**
 * @brief Get hostname from IP address (DNS lookup)
 */
bool resolve_hostname(const char* ip, char* hostname_buffer, size_t buffer_size) {
    if (!ip || !hostname_buffer || buffer_size == 0) return false;

    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;

    if (inet_pton(AF_INET, ip, &sa.sin_addr) <= 0) {
        return false;
    }

    char hbuf[NI_MAXHOST];
    int res = getnameinfo((struct sockaddr*)&sa, sizeof(sa), hbuf, sizeof(hbuf), NULL, 0, NI_NAMEREQD);
    if (res != 0) {
        LOG_NETWORK_WARNING("Failed to resolve hostname for %s: %s", ip, gai_strerror(res));
        return false;
    }

    if (strlen(hbuf) >= buffer_size) {
        return false;
    }

    strcpy(hostname_buffer, hbuf);
    return true;
}

/**
 * @brief Test network connectivity to a specific host
 */
bool test_connectivity(const char* host_ip, uint32_t timeout_ms) {
    if (!host_ip) return false;

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(TEXT_PORT);

    if (inet_pton(AF_INET, host_ip, &server_addr.sin_addr) <= 0) {
        return false;
    }

    int sock = create_tcp_socket();
    if (sock < 0) {
        return false;
    }

    if (!set_socket_timeout(sock, timeout_ms)) {
        close(sock);
        return false;
    }

    int result = connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    close(sock);

    return (result == 0);
}

/**
 * @brief Get network interface information
 */
bool get_network_interface_info(char* interface_name, size_t name_size, uint8_t* mac_address) {
    if (!interface_name || !mac_address || name_size == 0) return false;

    struct ifaddrs* interfaces = NULL;
    if (getifaddrs(&interfaces) != 0) {
        return false;
    }

    bool found = false;
    for (struct ifaddrs* ifa = interfaces; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;

        if (ifa->ifa_addr->sa_family == AF_INET && !(ifa->ifa_flags & IFF_LOOPBACK)) {
            // Get interface name
            if (strlen(ifa->ifa_name) < name_size) {
                strcpy(interface_name, ifa->ifa_name);

                // Get MAC address
                int sock = socket(AF_INET, SOCK_DGRAM, 0);
                if (sock >= 0) {
                    struct ifreq ifr;
                    memset(&ifr, 0, sizeof(ifr));
                    strcpy(ifr.ifr_name, ifa->ifa_name);

                    if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
                        memcpy(mac_address, ifr.ifr_hwaddr.sa_data, 6);
                        found = true;
                    }
                    close(sock);
                }
                break;
            }
        }
    }

    freeifaddrs(interfaces);
    return found;
}

/**
 * @brief Enable/disable network debugging
 */
void network_set_debug(bool enable) {
    g_debug_enabled = enable;
    if (enable) {
        LOG_NETWORK_INFO("Network debugging enabled");
    }
}

// ============================================================================
// INTERNAL HELPER FUNCTIONS
// ============================================================================

/**
 * @brief Create a TCP socket with proper configuration
 */
static int create_tcp_socket(void) {
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        LOG_NETWORK_ERROR(ERROR_SOCKET_CREATE, "Failed to create TCP socket: %s", strerror(errno));
        return -1;
    }

    // Set socket options for better reliability
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));

    return sock;
}

/**
 * @brief Set socket timeout
 */
static int set_socket_timeout(int sock, int timeout_ms) {
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        return 0; // Non-fatal, continue without timeout
    }

    if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) {
        return 0; // Non-fatal, continue without timeout
    }

    return 1;
}

/**
 * @brief Validate IP address format using regex-like validation
 */
static bool is_valid_ip_format(const char* ip) {
    if (!ip) return false;

    int segments = 0;
    int digits = 0;
    bool valid_segment = false;

    while (*ip) {
        if (*ip >= '0' && *ip <= '9') {
            digits = digits * 10 + (*ip - '0');
            valid_segment = true;
        } else if (*ip == '.') {
            if (!valid_segment || digits > 255) return false;
            segments++;
            digits = 0;
            valid_segment = false;
        } else {
            return false;
        }
        ip++;
    }

    return (segments == 3 && valid_segment && digits <= 255);
}