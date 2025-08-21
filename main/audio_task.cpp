#include "include/audio_task.h"
#include "include/config.h"
#include "include/shared_data.h"
#include "bt_audio.h"
#include "esp_log.h"
#include "driver/i2s.h"
#include "opus.h"
#include "HaLowMeshManager.h"
#include "lwip/sockets.h"
#include <math.h>

// I2S Configuration
#define I2S_SAMPLE_RATE     (16000)
#define I2S_NUM             (I2S_NUM_0)
#define I2S_BCK_PIN         (PIN_I2S_BCLK)
#define I2S_LRC_PIN         (PIN_I2S_LRC)
#define I2S_DO_PIN          (PIN_I2S_DOUT)
#define I2S_DI_PIN          (PIN_I2S_DIN)

// Opus Configuration
#define OPUS_FRAME_SIZE     (320) // 16000 Hz * 20ms
#define OPUS_BITRATE        (24000)
#define MAX_PACKET_SIZE     (1500)

static void init_i2s() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX),
        .sample_rate = I2S_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT, // For MEMS mic
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 256,
        .use_apll = false,
        .tx_desc_auto_clear = true
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCK_PIN,
        .ws_io_num = I2S_LRC_PIN,
        .data_out_num = I2S_DO_PIN,
        .data_in_num = I2S_DI_PIN
    };

    ESP_ERROR_CHECK(i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL));
    ESP_ERROR_CHECK(i2s_set_pin(I2S_NUM, &pin_config));
    ESP_LOGI(TAG, "I2S driver installed.");
}

#define OVER_SOUND_FREQ 440.0f // A4 tone
#define OVER_SOUND_DUR_MS 100
#define OVER_SOUND_AMPLITUDE 5000 // Amplitude (out of 32767 for int16_t)

static void play_over_sound() {
    ESP_LOGI(TAG, "Playing 'over' sound...");

    // Calculate number of samples
    const int num_samples = (int)((float)I2S_SAMPLE_RATE * (float)OVER_SOUND_DUR_MS / 1000.0f);
    int16_t* buffer = (int16_t*)malloc(num_samples * sizeof(int16_t));
    if (!buffer) {
        ESP_LOGE(TAG, "Failed to allocate buffer for over sound");
        return;
    }

    // Generate sine wave
    for (int i = 0; i < num_samples; i++) {
        float t = (float)i / (float)I2S_SAMPLE_RATE;
        buffer[i] = (int16_t)(OVER_SOUND_AMPLITUDE * sin(2.0f * M_PI * OVER_SOUND_FREQ * t));
    }

    // Write to I2S
    size_t bytes_written;
    i2s_write(I2S_NUM, buffer, num_samples * sizeof(int16_t), &bytes_written, portMAX_DELAY);

    // After playing the tone, it's good practice to write some silence
    // to ensure the buffer clears and prevent any lingering noise.
    i2s_zero_dma_buffer(I2S_NUM);

    free(buffer);
    ESP_LOGI(TAG, "'Over' sound finished.");
}


void audioTask(void *pvParameters) {
    ESP_LOGI(TAG, "audioTask started");

    // Initialize I2S
    init_i2s();

    // TODO: Initialize GPIO for the PTT button
    // gpio_pad_select_gpio(PIN_BUTTON_PTT);
    // gpio_set_direction(PIN_BUTTON_PTT, GPIO_MODE_INPUT);
    // gpio_pullup_en(PIN_BUTTON_PTT);

    // TODO: Initialize Opus Encoder
    // int opus_error;
    // OpusEncoder *encoder = opus_encoder_create(I2S_SAMPLE_RATE, 1, OPUS_APPLICATION_VOIP, &opus_error);
    // if (opus_error != OPUS_OK) {
    //     ESP_LOGE(TAG, "Failed to create Opus encoder: %d", opus_error);
    // }
    // opus_encoder_ctl(encoder, OPUS_SET_BITRATE(OPUS_BITRATE));

    // TODO: Initialize Opus Decoder
    // OpusDecoder *decoder = opus_decoder_create(I2S_SAMPLE_RATE, 1, &opus_error);

    // Create a non-blocking UDP socket for receiving audio
    int rx_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (rx_sock < 0) {
        ESP_LOGE(TAG, "Unable to create RX socket: errno %d", errno);
        vTaskDelete(NULL);
    }
    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(VOICE_PORT);
    int err = bind(rx_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err < 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
    }
    fcntl(rx_sock, F_SETFL, O_NONBLOCK);

    bool is_transmitting = false;

    // Main audio loop
    for(;;) {
        // Check for commands from the UI task
        audio_command_t cmd;
        if (xQueueReceive(audio_command_queue, &cmd, (TickType_t)0) == pdPASS) {
            if (cmd == AUDIO_CMD_START_TX) {
                is_transmitting = true;
                ESP_LOGI(TAG, "Audio task started transmitting.");
            } else if (cmd == AUDIO_CMD_STOP_TX) {
                is_transmitting = false;
                ESP_LOGI(TAG, "Audio task stopped transmitting.");
                play_over_sound();
            }
        }

        if (is_bt_audio_connected()) {
            // Bluetooth headset is connected
            if (is_transmitting) {
                // TX LOGIC: Read from BT mic and send to mesh
                uint8_t bt_mic_buf[512];
                int bytes_read = bt_audio_read_mic_data(bt_mic_buf, sizeof(bt_mic_buf));
                if (bytes_read > 0) {
                    HaLowMeshManager::getInstance().sendUdpMulticast(bt_mic_buf, bytes_read, VOICE_PORT);
                    ESP_LOGD(TAG, "Transmitted %d audio bytes from BT", bytes_read);
                }
            } else {
                // RX LOGIC: Receive from mesh and play on BT speaker
                uint8_t rx_buf[MAX_PACKET_SIZE];
                int len = recv(rx_sock, rx_buf, sizeof(rx_buf), 0);
                if (len > 0) {
                    bt_audio_send_data(rx_buf, len);
                    ESP_LOGD(TAG, "Received and sent %d audio bytes to BT", len);
                }
            }
        } else {
            // No BT headset, use I2S
            if (is_transmitting) {
                // TX LOGIC
                int16_t i2s_buffer[OPUS_FRAME_SIZE];
                size_t bytes_read;
                i2s_read(I2S_NUM, i2s_buffer, sizeof(i2s_buffer), &bytes_read, portMAX_DELAY);

                if (bytes_read > 0) {
                    // In a real implementation, you would use the opus encoder here.
                    // For this stub, we will just send the raw data as a placeholder.
                    HaLowMeshManager::getInstance().sendUdpMulticast((const uint8_t*)i2s_buffer, bytes_read, VOICE_PORT);
                    ESP_LOGD(TAG, "Transmitted %d audio bytes from I2S", bytes_read);
                }
            } else {
                // RX LOGIC
                uint8_t rx_buf[MAX_PACKET_SIZE];
                int len = recv(rx_sock, rx_buf, sizeof(rx_buf), 0);
                if (len > 0) {
                    // In a real implementation, you would decode the opus data here.
                    // For this stub, we just assume the received data is raw PCM and play it.
                    // A real jitter buffer would be needed here.
                    size_t bytes_written;
                    i2s_write(I2S_NUM, rx_buf, len, &bytes_written, portMAX_DELAY);
                    ESP_LOGD(TAG, "Received and played %d audio bytes on I2S", bytes_written);
                }
            }
        }

        // Delay to allow other tasks to run
        vTaskDelay(pdMS_TO_TICKS(20)); // A 20ms delay is typical for voice packets
    }
}
