#ifndef BT_AUDIO_H
#define BT_AUDIO_H

#include <stdbool.h>
#include <stdint.h>
#include <vector>
#include "esp_bt_defs.h"

// This struct holds information about a discovered Bluetooth device.
// It's defined here so the UI task can use it.
typedef struct {
    char name[249]; // ESP_BT_GAP_MAX_BDNAME_LEN
    esp_bd_addr_t bda;
} bt_device_t;


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the Bluetooth audio component.
 */
void bt_audio_init(void);

/**
 * @brief Checks if a Bluetooth audio device is connected and audio is active.
 * @return True if Bluetooth audio is active, false otherwise.
 */
bool is_bt_audio_connected(void);

/**
 * @brief Sends audio data to the connected Bluetooth headset.
 * @param data Pointer to the audio data buffer.
 * @param len Length of the audio data in bytes.
 */
void bt_audio_send_data(const uint8_t *data, uint16_t len);

/**
 * @brief Reads audio data from the connected Bluetooth headset's microphone.
 *        This function is non-blocking.
 * @param buf Pointer to the buffer to store the audio data.
 * @param max_len Maximum length of the buffer.
 * @return The number of bytes read, or 0 if no data is available.
 */
int bt_audio_read_mic_data(uint8_t *buf, int max_len);

/**
 * @brief Starts discovery of nearby Bluetooth devices.
 *        The discovery will run for 10 seconds.
 */
void bt_audio_start_discovery(void);

/**
 * @brief Gets the list of discovered Bluetooth devices.
 * @note This function is not thread-safe and should be called from the UI task.
 *       A better implementation would use a mutex.
 * @return A copy of the vector of discovered devices.
 */
std::vector<bt_device_t> bt_audio_get_discovered_devices(void);

/**
 * @brief Connects to a Bluetooth device by its address.
 * @param bda The Bluetooth device address.
 */
void bt_audio_connect(const esp_bd_addr_t bda);


#ifdef __cplusplus
}
#endif

#endif // BT_AUDIO_H
