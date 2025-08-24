#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h" // For mutexes
#include <stdbool.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <map>

// C++ equivalent of the ServiceType enum in the .proto file.
// This allows other components to use the type without a direct protobuf dependency.
enum class ServiceType {
    UNSPECIFIED = 0,
    ATAK_GATEWAY = 1,
    INTERNET_GATEWAY = 2,
    AUDIO_REPEATER = 3,
};

// Structure to hold info about a discovered teammate
struct MeshNodeInfo {
    std::string callsign;
    std::string node_id; // Using node_id as the primary key
    std::string ipAddress;
    std::vector<ServiceType> services;
};

// Structure to hold link quality and statistics for a peer node
struct PeerLinkStats {
    int32_t rssi; // As seen by our node
    int32_t snr; // As seen by our node
    int32_t remote_rssi; // As reported by the peer
    int32_t remote_snr; // As reported by the peer
    uint32_t last_latency_ms; // Round-trip time of last health check
    uint32_t last_update_time; // Timestamp of the last health update
};

// Structure to hold tactical info about a teammate
struct TeammateInfo {
    std::string callsign;
    std::string node_id;
    double lat;
    double lon;
    uint32_t last_update_time;
};


// A structure to hold status updates for the UI
typedef struct {
    bool has_gps_lock;
    uint8_t contact_count; // This can be removed if we are sharing the whole list
} ui_update_t;

// A queue to send status updates from other tasks to the UI task
extern QueueHandle_t ui_update_queue;

// A shared map to hold the list of contacts, keyed by node_id, protected by a mutex
extern std::map<std::string, MeshNodeInfo> g_contact_list;
extern SemaphoreHandle_t g_contact_list_mutex;

// A shared map to hold the link statistics for each peer, keyed by node_id
extern std::map<std::string, PeerLinkStats> g_peer_link_stats;
extern SemaphoreHandle_t g_peer_link_stats_mutex;

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

// A shared vector to hold the tactical information of all teammates
extern std::vector<TeammateInfo> g_teammate_locations;
extern SemaphoreHandle_t g_teammate_locations_mutex;


void shared_data_init();

// Queue helper functions with overflow handling
BaseType_t send_ui_update(const ui_update_t* update);
BaseType_t send_outgoing_message(const outgoing_message_t* message);
BaseType_t send_audio_command(const audio_command_t* command);
BaseType_t send_incoming_message(const incoming_message_t* message);

// Queue status monitoring functions
UBaseType_t get_ui_update_queue_spaces(void);
UBaseType_t get_outgoing_message_queue_spaces(void);
UBaseType_t get_audio_command_queue_spaces(void);
UBaseType_t get_incoming_message_queue_spaces(void);

UBaseType_t get_ui_update_queue_size(void);
UBaseType_t get_outgoing_message_queue_size(void);
UBaseType_t get_audio_command_queue_size(void);
UBaseType_t get_incoming_message_queue_size(void);

#endif // SHARED_DATA_H
