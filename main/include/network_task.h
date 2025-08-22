#ifndef NETWORK_TASK_H
#define NETWORK_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// NETWORK TASK FUNCTIONS
// ============================================================================

/**
 * @brief Network discovery and communication task
 *
 * This task handles network discovery, mesh management, and communication
 * with other AirCom nodes.
 *
 * @param pvParameters FreeRTOS task parameters
 */
void networkTask(void *pvParameters);

/**
 * @brief TCP server task for receiving messages
 *
 * This task listens for incoming TCP connections and processes received messages.
 *
 * @param pvParameters FreeRTOS task parameters
 */
void tcp_server_task(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif // NETWORK_TASK_H