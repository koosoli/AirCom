#include "include/bt_audio.h"
#include "config.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_hf_client_api.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <atomic>
#include <vector>
#include <mutex>

static const char* TAG = "BT_AUDIO";

// State
static std::atomic<bool> s_bt_audio_connected(false);
static esp_hf_sync_conn_hdl_t s_sync_conn_hdl = 0;

// Discovered devices
static std::vector<bt_device_t> s_discovered_devices;
static std::mutex s_devices_mutex;

// Mic queue
#define MIC_QUEUE_LEN 10
#define MIC_BUF_SIZE 512
typedef struct {
    uint8_t data[MIC_BUF_SIZE];
    uint16_t len;
} audio_packet_t;
static QueueHandle_t s_mic_queue = NULL;

// Forward declarations
static void bt_app_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param);
static void bt_app_hf_client_cb(esp_hf_client_cb_event_t event, esp_hf_client_cb_param_t *param);
static void bt_in_stream_cb(esp_hf_sync_conn_hdl_t sync_conn_hdl, esp_hf_audio_buff_t *audio_buf, bool is_bad_frame);

void bt_audio_init(void) {
    esp_err_t ret;

    s_mic_queue = xQueueCreate(MIC_QUEUE_LEN, sizeof(audio_packet_t));
    if (!s_mic_queue) {
        ESP_LOGE(TAG, "Failed to create mic queue");
    }

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK) {
        ESP_LOGE(TAG, "%s initialize controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK) {
        ESP_LOGE(TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bluedroid_init()) != ESP_OK) {
        ESP_LOGE(TAG, "%s initialize bluedroid failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bluedroid_enable()) != ESP_OK) {
        ESP_LOGE(TAG, "%s enable bluedroid failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bt_gap_register_callback(bt_app_gap_cb)) != ESP_OK) {
        ESP_LOGE(TAG, "%s gap register failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_hf_client_init()) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize HFP client: %s", esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_hf_client_register_callback(bt_app_hf_client_cb)) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register HFP client callback: %s", esp_err_to_name(ret));
        return;
    }

    esp_hf_client_register_audio_data_callback(bt_in_stream_cb);

    // Set device name
    esp_bt_dev_set_device_name(CALLSIGN);

    // Set discoverable and connectable mode
    esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);


    ESP_LOGI(TAG, "Bluetooth audio component initialized");
}

bool is_bt_audio_connected(void) {
    return s_bt_audio_connected.load();
}

void bt_audio_send_data(const uint8_t *data, uint16_t len) {
    if (!is_bt_audio_connected()) {
        return;
    }

    esp_hf_audio_buff_t *audio_buf = esp_hf_client_audio_buff_alloc(len);
    if (audio_buf) {
        memcpy(audio_buf->data, data, len);
        audio_buf->len = len;
        esp_hf_client_audio_data_send(s_sync_conn_hdl, audio_buf);
    } else {
        ESP_LOGE(TAG, "Failed to allocate audio buffer for sending");
    }
}

int bt_audio_read_mic_data(uint8_t *buf, int max_len) {
    if (s_mic_queue) {
        audio_packet_t packet;
        if (xQueueReceive(s_mic_queue, &packet, 0) == pdPASS) {
            int len_to_copy = (packet.len > max_len) ? max_len : packet.len;
            memcpy(buf, packet.data, len_to_copy);
            return len_to_copy;
        }
    }
    return 0;
}

void bt_audio_start_discovery(void) {
    ESP_LOGI(TAG, "Starting device discovery...");
    {
        std::lock_guard<std::mutex> lock(s_devices_mutex);
        s_discovered_devices.clear();
    }
    esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 10, 0);
}

std::vector<bt_device_t> bt_audio_get_discovered_devices(void) {
    std::lock_guard<std::mutex> lock(s_devices_mutex);
    return s_discovered_devices;
}

void bt_audio_connect(const esp_bd_addr_t bda) {
    ESP_LOGI(TAG, "Connecting to device...");
    esp_hf_client_connect(bda);
}


static void bt_in_stream_cb(esp_hf_sync_conn_hdl_t sync_conn_hdl, esp_hf_audio_buff_t *audio_buf, bool is_bad_frame) {
    if (is_bad_frame || audio_buf == NULL) {
        if (audio_buf) {
            esp_hf_client_audio_buff_free(audio_buf);
        }
        return;
    }

    if (s_mic_queue && audio_buf->len > 0) {
        if (audio_buf->len <= MIC_BUF_SIZE) {
            audio_packet_t packet;
            memcpy(packet.data, audio_buf->data, audio_buf->len);
            packet.len = audio_buf->len;
            if (xQueueSend(s_mic_queue, &packet, (TickType_t)0) != pdPASS) {
                // ESP_LOGW(TAG, "Mic queue full");
            }
        } else {
            ESP_LOGW(TAG, "Received audio packet larger than buffer");
        }
    }
    esp_hf_client_audio_buff_free(audio_buf);
}


static void bt_app_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param) {
    switch (event) {
    case ESP_BT_GAP_DISC_RES_EVT: {
        ESP_LOGI(TAG, "GAP Discovery Result");
        bt_device_t new_device;
        memcpy(new_device.bda, param->disc_res.bda, ESP_BD_ADDR_LEN);

        char *name = (char*)param->disc_res.name;
        if(name) {
            strncpy(new_device.name, name, sizeof(new_device.name) - 1);
        } else {
            snprintf(new_device.name, sizeof(new_device.name), "Device %02x:%02x:%02x", new_device.bda[3], new_device.bda[4], new_device.bda[5]);
        }

        std::lock_guard<std::mutex> lock(s_devices_mutex);
        bool found = false;
        for (const auto& device : s_discovered_devices) {
            if (memcmp(device.bda, new_device.bda, ESP_BD_ADDR_LEN) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            s_discovered_devices.push_back(new_device);
        }
        break;
    }
    case ESP_BT_GAP_DISC_STATE_CHANGED_EVT:
        ESP_LOGI(TAG, "GAP Discovery State Changed: %s",
                 param->disc_st_chg.state == ESP_BT_GAP_DISCOVERY_STOPPED ? "STOPPED" : "STARTED");
        break;
    case ESP_BT_GAP_RMT_SRVCS_EVT:
        ESP_LOGI(TAG, "GAP Remote Services");
        break;
    case ESP_BT_GAP_RMT_SRVC_REC_EVT:
        ESP_LOGI(TAG, "GAP Remote Service Record");
        break;
    case ESP_BT_GAP_AUTH_CMPL_EVT: {
        if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
            ESP_LOGI(TAG, "authentication success: %s", param->auth_cmpl.device_name);
            esp_log_buffer_hex(TAG, param->auth_cmpl.bda, ESP_BD_ADDR_LEN);
        } else {
            ESP_LOGE(TAG, "authentication failed, status:%d", param->auth_cmpl.stat);
        }
        break;
    }
    default:
        ESP_LOGI(TAG, "GAP event %d", event);
        break;
    }
}

static void bt_app_hf_client_cb(esp_hf_client_cb_event_t event, esp_hf_client_cb_param_t *param) {
    switch (event) {
    case ESP_HF_CLIENT_CONNECTION_STATE_EVT:
        ESP_LOGI(TAG, "HFP Connection State: %d, peer_feat: %d, chld_feat: %d",
                 param->conn_stat.state, param->conn_stat.peer_feat, param->conn_stat.chld_feat);
        if (param->conn_stat.state == ESP_HF_CLIENT_CONNECTION_STATE_SLC_CONNECTED) {
            // SLC connected, now we can connect audio
            ESP_LOGI(TAG, "SLC connected. Connecting audio...");
            esp_hf_client_connect_audio(param->conn_stat.remote_bda);
        }
        break;
    case ESP_HF_CLIENT_AUDIO_STATE_EVT:
        ESP_LOGI(TAG, "HFP Audio State: %d", param->audio_stat.state);
        if (param->audio_stat.state == ESP_HF_CLIENT_AUDIO_STATE_CONNECTED ||
            param->audio_stat.state == ESP_HF_CLIENT_AUDIO_STATE_CONNECTED_MSBC) {
            s_bt_audio_connected = true;
            s_sync_conn_hdl = param->audio_stat.sync_conn_handle;
            ESP_LOGI(TAG, "Bluetooth audio connected.");
        } else if (param->audio_stat.state == ESP_HF_CLIENT_AUDIO_STATE_DISCONNECTED) {
            s_bt_audio_connected = false;
            s_sync_conn_hdl = 0;
            ESP_LOGI(TAG, "Bluetooth audio disconnected.");
        }
        break;
    case ESP_HF_CLIENT_BVRA_EVT:
        ESP_LOGI(TAG, "HFP BVRA: %d", param->bvra.value);
        break;
    default:
        ESP_LOGI(TAG, "HFP event %d", event);
        break;
    }
}
