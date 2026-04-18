#ifndef RILP_H
#define RILP_H

#include <stdint.h>
#include <sys/socket.h>

#define RILP_VERSION 0x01
#define RILP_HEADER_SIZE 16

/* Layer identifiers (1-6) */
#define RILP_LAYER_KERNEL      1
#define RILP_LAYER_HAL         2
#define RILP_LAYER_MIDDLEWARE  3
#define RILP_LAYER_AI          4
#define RILP_LAYER_SECURITY    5
#define RILP_LAYER_FLEET       6
#define RILP_LAYER_BROADCAST   0

/* Message types */
#define RILP_MSG_ANNOUNCE       0x01
#define RILP_MSG_HEARTBEAT      0x02
#define RILP_MSG_SENSOR_DATA    0x03
#define RILP_MSG_ACTUATOR_CMD   0x04
#define RILP_MSG_AI_RESULT      0x05
#define RILP_MSG_SECURITY_ALERT 0x06
#define RILP_MSG_OTA_REQUEST    0x07
#define RILP_MSG_FLEET_STATUS   0x08
#define RILP_MSG_DIAG_QUERY     0x09
#define RILP_MSG_EMERGENCY      0xFF

/* RILP message header (16 bytes, packed) */
typedef struct __attribute__((packed)) {
    uint8_t  version;      /* RILP_VERSION */
    uint8_t  src_layer;    /* 1-6 */
    uint8_t  dst_layer;    /* 1-6, or 0 for broadcast */
    uint8_t  msg_type;     /* RILP_MSG_* */
    uint32_t seq_id;       /* sequence number */
    uint32_t deadline_us;  /* max latency in microseconds */
    uint32_t payload_len;  /* payload size in bytes */
} rilp_header_t;

/* Full RILP message (header + payload) */
typedef struct {
    rilp_header_t header;
    uint8_t* payload;
    size_t total_len;
} rilp_message_t;

/* Initialize a message header */
void rilp_header_init(rilp_header_t* hdr, 
                      uint8_t src, uint8_t dst, 
                      uint8_t msg_type, uint32_t deadline_us);

/* Serialize header to bytes */
void rilp_header_serialize(const rilp_header_t* hdr, uint8_t* buf);

/* Deserialize bytes to header */
void rilp_header_deserialize(const uint8_t* buf, rilp_header_t* hdr);

/* Create a complete message */
rilp_message_t* rilp_message_new(uint8_t src, uint8_t dst,
                                  uint8_t msg_type, uint32_t deadline_us,
                                  const uint8_t* payload, uint32_t payload_len);

/* Free a message */
void rilp_message_free(rilp_message_t* msg);

/* Send a message over a socket */
int rilp_send(int sockfd, const rilp_message_t* msg);

/* Receive a message from a socket */
rilp_message_t* rilp_recv(int sockfd);

/* Get string description of message type */
const char* rilp_msg_type_str(uint8_t msg_type);

/* Get string description of layer */
const char* rilp_layer_str(uint8_t layer);

#endif /* RILP_H */

/* Socket transport functions */
int rilp_create_server(const char* socket_path);
int rilp_accept_connection(int server_sock);
int rilp_connect(const char* socket_path);
void rilp_close(int sockfd);
