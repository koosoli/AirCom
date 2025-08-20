#include "include/gps_task.h"
#include "include/config.h"
#include "include/shared_data.h"
#include "driver/uart.h"
#include "esp_log.h"

// The GPS object
static TinyGPSPlus gps;

// The global data structure to hold GPS data
static GPSData gpsData;

// Mutex for protecting access to the gpsData struct
static portMUX_TYPE gpsDataMutex = portMUX_INITIALIZER_UNLOCKED;

// UART buffer
static const int RX_BUF_SIZE = 1024;

void init_uart() {
    const uart_config_t uart_config = {
        .baud_rate = GPS_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    // We won't use a buffer for sending data.
    uart_driver_install(GPS_UART_NUM, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(GPS_UART_NUM, &uart_config);
    uart_set_pin(GPS_UART_NUM, PIN_GPS_TX, PIN_GPS_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

GPSData gps_get_data() {
    GPSData data;
    portENTER_CRITICAL(&gpsDataMutex);
    data = gpsData;
    portEXIT_CRITICAL(&gpsDataMutex);
    return data;
}

void gpsTask(void *pvParameters) {
    ESP_LOGI(TAG, "gpsTask started");
    init_uart();

    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE);

    bool last_valid_state = false;

    for (;;) {
        const int rxBytes = uart_read_bytes(GPS_UART_NUM, data, RX_BUF_SIZE, 100 / portTICK_PERIOD_MS);
        if (rxBytes > 0) {
            for (int i = 0; i < rxBytes; i++) {
                if (gps.encode(data[i])) {
                    // A new sentence has been parsed
                    bool current_valid_state = gps.location.isValid();

                    portENTER_CRITICAL(&gpsDataMutex);
                    gpsData.isValid = current_valid_state;
                    if (gpsData.isValid) {
                        gpsData.latitude = gps.location.lat();
                        gpsData.longitude = gps.location.lng();
                        gpsData.altitude = gps.altitude.meters();
                        gpsData.speed = gps.speed.mps();
                        gpsData.satellites = gps.satellites.value();
                        if (gps.date.isValid()) {
                            gpsData.date = gps.date.value();
                        }
                        if (gps.time.isValid()) {
                            gpsData.time = gps.time.value();
                        }
                    }
                    portEXIT_CRITICAL(&gpsDataMutex);

                    // If validity changed, send an update to the UI task
                    if (current_valid_state != last_valid_state) {
                        last_valid_state = current_valid_state;
                        ui_update_t update = { .has_gps_lock = current_valid_state, .contact_count = 0xFF }; // 0xFF means no change
                        xQueueSend(ui_update_queue, &update, (TickType_t)0);
                    }
                }
            }
        }
    }
    free(data);
}
