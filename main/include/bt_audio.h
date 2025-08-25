#ifndef BT_AUDIO_H
#define BT_AUDIO_H

// Bluetooth Audio Service - Stub Implementation
// This is a placeholder for Bluetooth audio functionality

/**
 * @brief Initialize the Bluetooth audio service
 * @return 0 on success, error code on failure
 */
int bt_audio_init(void);

/**
 * @brief Start Bluetooth audio streaming
 * @return 0 on success, error code on failure
 */
int bt_audio_start(void);

/**
 * @brief Stop Bluetooth audio streaming
 * @return 0 on success, error code on failure
 */
int bt_audio_stop(void);

/**
 * @brief Check if Bluetooth device is connected
 * @return 1 if connected, 0 if not connected
 */
int bt_audio_is_connected(void);

#endif // BT_AUDIO_H