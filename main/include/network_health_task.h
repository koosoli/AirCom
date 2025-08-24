#ifndef NETWORK_HEALTH_TASK_H
#define NETWORK_HEALTH_TASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/**
 * @brief Network Health Task
 *
 * This task is responsible for periodically broadcasting network health information,
 * such as RSSI, to other nodes in the mesh. This allows for dynamic link quality
 * assessment.
 *
 * @param pvParameters Task parameters (unused).
 */
void network_health_task(void *pvParameters);

#endif // NETWORK_HEALTH_TASK_H
