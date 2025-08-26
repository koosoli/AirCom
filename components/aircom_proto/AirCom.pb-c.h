#ifndef AIRCOM_PB_C_H
#define AIRCOM_PB_C_H

#include <cstdint>
#include <cstddef>

// Dummy structs
typedef struct _AirComPacket AirComPacket;
typedef struct _NodeInfo NodeInfo;
typedef struct _TextMessage TextMessage;
typedef struct _NetworkHealth NetworkHealth;

struct _AirComPacket {
    int payload_variant_case;
    NodeInfo* node_info;
    TextMessage* text_message;
    NetworkHealth* network_health;
    char* from_node;
    char* cot_message;
};

struct _NodeInfo {
    char* callsign;
    char* node_id;
};

struct _TextMessage {
    char* text;
};

struct _NetworkHealth {
    int rssi;
};

#define AIR_COM_PACKET__INIT {0,0,0,0,0,0}
#define NODE_INFO__INIT {0,0}
#define TEXT_MESSAGE__INIT {0}
#define AIR_COM_PACKET__PAYLOAD_VARIANT_NODE_INFO 1
#define AIR_COM_PACKET__PAYLOAD_VARIANT_TEXT_MESSAGE 2
#define AIR_COM_PACKET__PAYLOAD_VARIANT_NETWORK_HEALTH 3
#define AIR_COM_PACKET__PAYLOAD_VARIANT_COT_MESSAGE 4

// Dummy function prototypes
size_t air_com_packet__get_packed_size(const AirComPacket*);
void air_com_packet__pack(const AirComPacket*, uint8_t*);
AirComPacket* air_com_packet__unpack(void*, size_t, const uint8_t*);
void air_com_packet__free_unpacked(AirComPacket*, void*);

#endif // AIRCOM_PB_C_H
