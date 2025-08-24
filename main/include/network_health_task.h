#ifndef NETWORK_HEALTH_TASK_H
#define NETWORK_HEALTH_TASK_H

#include "freertos/FreeRTOS.h"

/**
 * @brief The main function for the Network Health Task.
 *
 * This task is responsible for the following:
 * - Periodically sending out HealthCheckRequest packets to all known peers.
 * - Receiving HealthCheckResponse packets and updating the g_peer_link_stats map.
 * - Processing incoming HealthCheckRequest packets and sending responses.
 * - Pruning stale entries from the stats map.
 *
 * @param pvParameters Unused.
 */
void network_health_task(void *pvParameters);

#endif // NETWORK_HEALTH_TASK_H
