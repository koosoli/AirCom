#include "include/shared_data.h"
#include "esp_log.h"

// Define the global variables
QueueHandle_t ui_update_queue;
QueueHandle_t outgoing_message_queue;
QueueHandle_t audio_command_queue;
QueueHandle_t incoming_message_queue;
std::map<std::string, MeshNodeInfo> g_contact_list;
SemaphoreHandle_t g_contact_list_mutex;
std::map<std::string, PeerLinkStats> g_peer_link_stats;
SemaphoreHandle_t g_peer_link_stats_mutex;
std::vector<TeammateInfo> g_teammate_locations;
SemaphoreHandle_t g_teammate_locations_mutex;

static const char* TAG = "SHARED_DATA";

// Queue size constants - increased to handle bursts and improve reliability
#define UI_UPDATE_QUEUE_SIZE 30
#define OUTGOING_MESSAGE_QUEUE_SIZE 20
#define AUDIO_COMMAND_QUEUE_SIZE 15
#define INCOMING_MESSAGE_QUEUE_SIZE 25

// Timeout for queue operations (in ticks) - increased for better reliability
#define QUEUE_SEND_TIMEOUT pdMS_TO_TICKS(200)
#define QUEUE_SEND_TIMEOUT_SHORT pdMS_TO_TICKS(50)
#define QUEUE_SEND_TIMEOUT_CRITICAL pdMS_TO_TICKS(10) // For critical audio operations

// Mutex timeout constants
#define MUTEX_TIMEOUT_DEFAULT pdMS_TO_TICKS(500)
#define MUTEX_TIMEOUT_SHORT pdMS_TO_TICKS(100)
#define MUTEX_TIMEOUT_CRITICAL pdMS_TO_TICKS(50)

void shared_data_init() {
    // Create a queue capable of holding UI update structures with overflow protection
    ui_update_queue = xQueueCreate(UI_UPDATE_QUEUE_SIZE, sizeof(ui_update_t));
    if (!ui_update_queue) {
        ESP_LOGE(TAG, "Failed to create UI update queue");
    }

    // Create a queue for outgoing messages with larger capacity
    outgoing_message_queue = xQueueCreate(OUTGOING_MESSAGE_QUEUE_SIZE, sizeof(outgoing_message_t));
    if (!outgoing_message_queue) {
        ESP_LOGE(TAG, "Failed to create outgoing message queue");
    }

    // Create a queue for audio commands
    audio_command_queue = xQueueCreate(AUDIO_COMMAND_QUEUE_SIZE, sizeof(audio_command_t));
    if (!audio_command_queue) {
        ESP_LOGE(TAG, "Failed to create audio command queue");
    }

    // Create a queue for incoming messages with larger capacity
    incoming_message_queue = xQueueCreate(INCOMING_MESSAGE_QUEUE_SIZE, sizeof(incoming_message_t));
    if (!incoming_message_queue) {
        ESP_LOGE(TAG, "Failed to create incoming message queue");
    }

    // Create a mutex for guarding access to the contact list.
    g_contact_list_mutex = xSemaphoreCreateMutex();

    // Create a mutex for guarding access to the peer link stats map.
    g_peer_link_stats_mutex = xSemaphoreCreateMutex();

    // Create a mutex for guarding access to the teammate locations list.
    g_teammate_locations_mutex = xSemaphoreCreateMutex();

    ESP_LOGI(TAG, "Shared data initialized with improved queue sizes");
}

// Queue helper functions with overflow handling and retry logic
BaseType_t send_ui_update(const ui_update_t* update) {
    if (!ui_update_queue) return pdFAIL;

    BaseType_t result = xQueueSend(ui_update_queue, update, QUEUE_SEND_TIMEOUT);
    if (result != pdPASS) {
        ESP_LOGW(TAG, "UI update queue full, dropping update (spaces: %d)",
                get_ui_update_queue_spaces());
    }
    return result;
}

BaseType_t send_outgoing_message(const outgoing_message_t* message) {
    if (!outgoing_message_queue) return pdFAIL;

    // Try with normal timeout first
    BaseType_t result = xQueueSend(outgoing_message_queue, message, QUEUE_SEND_TIMEOUT);
    if (result != pdPASS) {
        // Retry with shorter timeout - better than dropping critical messages
        result = xQueueSend(outgoing_message_queue, message, MUTEX_TIMEOUT_SHORT);
        if (result != pdPASS) {
            ESP_LOGW(TAG, "Outgoing message queue full, dropping message (spaces: %d)",
                    get_outgoing_message_queue_spaces());
        }
    }
    return result;
}

BaseType_t send_audio_command(const audio_command_t* command) {
    if (!audio_command_queue) return pdFAIL;

    // Audio commands are critical - use shorter timeout but don't drop easily
    BaseType_t result = xQueueSend(audio_command_queue, command, QUEUE_SEND_TIMEOUT_CRITICAL);
    if (result != pdPASS) {
        // Retry once with normal timeout for critical audio operations
        result = xQueueSend(audio_command_queue, command, MUTEX_TIMEOUT_SHORT);
        if (result != pdPASS) {
            ESP_LOGW(TAG, "Audio command queue full, dropping command (spaces: %d)",
                    get_audio_command_queue_spaces());
        }
    }
    return result;
}

BaseType_t send_incoming_message(const incoming_message_t* message) {
    if (!incoming_message_queue) return pdFAIL;

    BaseType_t result = xQueueSend(incoming_message_queue, message, QUEUE_SEND_TIMEOUT);
    if (result != pdPASS) {
        ESP_LOGW(TAG, "Incoming message queue full, dropping message (spaces: %d)",
                get_incoming_message_queue_spaces());
    }
    return result;
}

// Queue status monitoring functions
UBaseType_t get_ui_update_queue_spaces(void) {
    return uxQueueSpacesAvailable(ui_update_queue);
}

UBaseType_t get_outgoing_message_queue_spaces(void) {
    return uxQueueSpacesAvailable(outgoing_message_queue);
}

UBaseType_t get_audio_command_queue_spaces(void) {
    return uxQueueSpacesAvailable(audio_command_queue);
}

UBaseType_t get_incoming_message_queue_spaces(void) {
    return uxQueueSpacesAvailable(incoming_message_queue);
}

UBaseType_t get_ui_update_queue_size(void) {
    return UI_UPDATE_QUEUE_SIZE;
}

UBaseType_t get_outgoing_message_queue_size(void) {
    return OUTGOING_MESSAGE_QUEUE_SIZE;
}

UBaseType_t get_audio_command_queue_size(void) {
    return AUDIO_COMMAND_QUEUE_SIZE;
}

UBaseType_t get_incoming_message_queue_size(void) {
    return INCOMING_MESSAGE_QUEUE_SIZE;
}
