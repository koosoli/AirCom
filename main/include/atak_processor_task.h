#ifndef ATAK_PROCESSOR_TASK_H
#define ATAK_PROCESSOR_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// ATAK PROCESSOR TASK FUNCTIONS
// ============================================================================

/**
 * @brief ATAK processor task
 *
 * This task receives CoT messages from other clients on the network
 * and updates the shared list of teammate locations.
 *
 * @param pvParameters FreeRTOS task parameters
 */
void atak_processor_task(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif // ATAK_PROCESSOR_TASK_H