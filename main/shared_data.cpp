#include "include/shared_data.h"

// Define the global variables
QueueHandle_t ui_update_queue;
QueueHandle_t outgoing_message_queue;
QueueHandle_t audio_command_queue;
QueueHandle_t incoming_message_queue;
std::vector<MeshNodeInfo> g_contact_list;
SemaphoreHandle_t g_contact_list_mutex;
std::vector<TeammateInfo> g_teammate_locations;
SemaphoreHandle_t g_teammate_locations_mutex;

void shared_data_init() {
    // Create a queue capable of holding 5 ui_update_t structures.
    ui_update_queue = xQueueCreate(5, sizeof(ui_update_t));

    // Create a queue for outgoing messages
    outgoing_message_queue = xQueueCreate(5, sizeof(outgoing_message_t));

    // Create a queue for audio commands
    audio_command_queue = xQueueCreate(5, sizeof(audio_command_t));

    // Create a queue for incoming messages
    incoming_message_queue = xQueueCreate(5, sizeof(incoming_message_t));

    // Create a mutex for guarding access to the contact list.
    g_contact_list_mutex = xSemaphoreCreateMutex();

    // Create a mutex for guarding access to the teammate locations list.
    g_teammate_locations_mutex = xSemaphoreCreateMutex();
}
