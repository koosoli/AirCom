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
#include "freertos/task.h"
#include "esp_timer.h"

// I2S Configuration
#define I2S_SAMPLE_RATE     (16000)
#define I2S_NUM             (I2S_NUM_0)
#define I2S_BCK_PIN         (PIN_I2S_BCLK)
#define I2S_LRC_PIN         (PIN_I2S_LRC)
#define I2S_DO_PIN          (PIN_I2S_DOUT)
#define I2S_DI_PIN          (PIN_I2S_DIN)

// Audio Codec Configuration
#define AUDIO_FRAME_SIZE_MS (20) // 20ms frames for low latency
#define AUDIO_FRAME_SIZE_SAMPLES (I2S_SAMPLE_RATE * AUDIO_FRAME_SIZE_MS / 1000) // 320 samples
#define AUDIO_BITRATE        (24000) // 24 kbps for voice
#define AUDIO_MAX_PACKET_SIZE (1500) // Maximum compressed packet size
#define AUDIO_BT_MIC_BUFFER_SIZE (512) // Bluetooth microphone buffer size

// Audio timing configuration for real-time guarantees
#define AUDIO_FRAME_INTERVAL_US (AUDIO_FRAME_SIZE_MS * 1000) // 20ms in microseconds
#define AUDIO_WATCHDOG_TIMEOUT_US (AUDIO_FRAME_INTERVAL_US * 2.5) // 50ms watchdog timeout
#define AUDIO_MAX_PROCESSING_TIME_US (AUDIO_FRAME_INTERVAL_US * 0.75) // 15ms max processing time

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

// Audio tone configuration for "over" signal
#define AUDIO_OVER_TONE_FREQ_HZ 440.0f // A4 tone frequency
#define AUDIO_OVER_TONE_DURATION_MS 100 // Duration of over signal
#define AUDIO_OVER_TONE_AMPLITUDE 5000 // Amplitude (out of 32767 for int16_t)

// Performance monitoring constants
#define AUDIO_YIELD_INTERVAL 10 // Yield every 10 frames
#define AUDIO_LOG_INTERVAL_MS 1000 // Log statistics every second

static void play_over_sound() {
    ESP_LOGI(TAG, "Playing 'over' sound...");

    // Calculate number of samples
    const int num_samples = (int)((float)I2S_SAMPLE_RATE * (float)AUDIO_OVER_TONE_DURATION_MS / 1000.0f);
    int16_t* buffer = (int16_t*)malloc(num_samples * sizeof(int16_t));
    if (!buffer) {
        ESP_LOGE(TAG, "Failed to allocate buffer for over sound");
        return;
    }

    // Generate sine wave for "over" signal
    for (int i = 0; i < num_samples; i++) {
        float t = (float)i / (float)I2S_SAMPLE_RATE;
        buffer[i] = (int16_t)(AUDIO_OVER_TONE_AMPLITUDE * sin(2.0f * M_PI * AUDIO_OVER_TONE_FREQ_HZ * t));
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
    ESP_LOGI(TAG, "audioTask started with real-time performance optimizations");

    // Initialize I2S
    init_i2s();

    // Create a non-blocking UDP socket for receiving audio
    int rx_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (rx_sock < 0) {
        ESP_LOGE(TAG, "Unable to create RX socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }

    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(VOICE_PORT);
    int err = bind(rx_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err < 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        close(rx_sock);
        vTaskDelete(NULL);
        return;
    }
    fcntl(rx_sock, F_SETFL, O_NONBLOCK);

    bool is_transmitting = false;
    uint64_t last_frame_time = esp_timer_get_time();
    uint64_t frame_start_time = 0;
    uint32_t timing_violations = 0;

    // Main audio loop with precise timing control
    for(;;) {
        frame_start_time = esp_timer_get_time();

        // Check for timing violations and log performance issues
        uint64_t frame_duration = frame_start_time - last_frame_time;
        if (frame_duration > AUDIO_WATCHDOG_TIMEOUT_US) {
            timing_violations++;
            ESP_LOGW(TAG, "Audio timing violation: %llu us (violation #%lu)",
                    frame_duration, timing_violations);
        }

        // Check for commands from the UI task with higher priority processing
        audio_command_t cmd;
        if (xQueueReceive(audio_command_queue, &cmd, (TickType_t)0) == pdPASS) {
            if (cmd == AUDIO_CMD_START_TX) {
                is_transmitting = true;
                ESP_LOGI(TAG, "Audio task started transmitting with timing guarantees");
            } else if (cmd == AUDIO_CMD_STOP_TX) {
                is_transmitting = false;
                ESP_LOGI(TAG, "Audio task stopped transmitting");
                // Yield to allow UI task to respond immediately
                taskYIELD();
                play_over_sound();
            }
        }

        uint64_t processing_start = esp_timer_get_time();

        if (is_bt_audio_connected()) {
            // Bluetooth headset processing with timing optimization
            if (is_transmitting) {
                uint8_t bt_mic_buf[AUDIO_BT_MIC_BUFFER_SIZE];
                int bytes_read = bt_audio_read_mic_data(bt_mic_buf, sizeof(bt_mic_buf));
                if (bytes_read > 0) {
                    HaLowMeshManager::getInstance().sendUdpMulticast(bt_mic_buf, bytes_read, VOICE_PORT);
                    ESP_LOGD(TAG, "Transmitted %d audio bytes from BT", bytes_read);
                }
            } else {
                uint8_t rx_buf[AUDIO_MAX_PACKET_SIZE];
                int len = recv(rx_sock, rx_buf, sizeof(rx_buf), 0);
                if (len > 0) {
                    bt_audio_send_data(rx_buf, len);
                    ESP_LOGD(TAG, "Received and sent %d audio bytes to BT", len);
                }
            }
        } else {
            // I2S processing with optimized timing
            if (is_transmitting) {
                int16_t i2s_buffer[AUDIO_FRAME_SIZE_SAMPLES];
                size_t bytes_read = 0;
                esp_err_t ret = i2s_read(I2S_NUM, i2s_buffer, sizeof(i2s_buffer), &bytes_read, 0); // Non-blocking

                if (ret == ESP_OK && bytes_read > 0) {
                    HaLowMeshManager::getInstance().sendUdpMulticast((const uint8_t*)i2s_buffer, bytes_read, VOICE_PORT);
                    ESP_LOGD(TAG, "Transmitted %d audio bytes from I2S", bytes_read);
                }
            } else {
                uint8_t rx_buf[MAX_PACKET_SIZE];
                int len = recv(rx_sock, rx_buf, sizeof(rx_buf), 0);
                if (len > 0) {
                    size_t bytes_written = 0;
                    esp_err_t ret = i2s_write(I2S_NUM, rx_buf, len, &bytes_written, 0); // Non-blocking

                    if (ret == ESP_OK) {
                        ESP_LOGD(TAG, "Received and played %d audio bytes on I2S", bytes_written);
                    }
                }
            }
        }

        // Monitor processing time to ensure we meet real-time constraints
        uint64_t processing_time = esp_timer_get_time() - processing_start;
        if (processing_time > AUDIO_MAX_PROCESSING_TIME_US) {
            ESP_LOGW(TAG, "Audio processing exceeded limit: %llu us", processing_time);
        }

        // Precise timing control for next frame
        last_frame_time = frame_start_time;
        uint64_t target_next_frame = frame_start_time + AUDIO_FRAME_INTERVAL_US;
        uint64_t current_time = esp_timer_get_time();

        if (current_time < target_next_frame) {
            uint64_t sleep_time_us = target_next_frame - current_time;
            uint32_t sleep_ticks = pdUS_TO_TICKS(sleep_time_us);

            if (sleep_ticks > 0) {
                vTaskDelay(sleep_ticks);
            }
        } else {
            // We're behind schedule, yield to other tasks and try to catch up
            ESP_LOGD(TAG, "Audio frame behind schedule, yielding");
            taskYIELD();
        }

        // Yield periodically to maintain system responsiveness
        static uint32_t yield_counter = 0;
        if (++yield_counter % AUDIO_YIELD_INTERVAL == 0) {
            taskYIELD();
        }
    }

    // Cleanup
    close(rx_sock);
    vTaskDelete(NULL);
}
