#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h" // For mutexes
#include <stdbool.h>
#include <stdint.h>
#include <vector>
#include <string>

// Structure to hold info about a discovered teammate
struct MeshNodeInfo {
    std::string callsign; // Will be extracted from discovery JSON
    std::string ipAddress;
};


// A structure to hold status updates for the UI
typedef struct {
    bool has_gps_lock;
    uint8_t contact_count; // This can be removed if we are sharing the whole list
} ui_update_t;

// A queue to send status updates from other tasks to the UI task
extern QueueHandle_t ui_update_queue;

// A shared vector to hold the list of contacts, protected by a mutex
extern std::vector<MeshNodeInfo> g_contact_list;
extern SemaphoreHandle_t g_contact_list_mutex;

// A structure to hold an outgoing text message
typedef struct {
    char target_ip[40]; // IPv6 addresses can be long
    std::vector<uint8_t> encrypted_payload;
} outgoing_message_t;

// A queue for the UI task to send outgoing messages to the network task
extern QueueHandle_t outgoing_message_queue;

// Enum for audio commands
typedef enum {
    AUDIO_CMD_START_TX,
    AUDIO_CMD_STOP_TX,
} audio_command_t;

// A queue for the UI task to send commands to the audio task
extern QueueHandle_t audio_command_queue;

// A structure to hold an incoming text message for the UI
typedef struct {
    std::string sender_callsign;
    std::string message_text;
} incoming_message_t;

// A queue for the network task to send incoming messages to the UI task
extern QueueHandle_t incoming_message_queue;


void shared_data_init();


#endif // SHARED_DATA_H
