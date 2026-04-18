#include "rilp.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>

void rilp_header_init(rilp_header_t* hdr, 
                      uint8_t src, uint8_t dst, 
                      uint8_t msg_type, uint32_t deadline_us) {
    static uint32_t seq_counter = 0;
    
    hdr->version = RILP_VERSION;
    hdr->src_layer = src;
    hdr->dst_layer = dst;
    hdr->msg_type = msg_type;
    hdr->seq_id = seq_counter++;
    hdr->deadline_us = deadline_us;
    hdr->payload_len = 0;
}

void rilp_header_serialize(const rilp_header_t* hdr, uint8_t* buf) {
    buf[0] = hdr->version;
    buf[1] = hdr->src_layer;
    buf[2] = hdr->dst_layer;
    buf[3] = hdr->msg_type;
    *(uint32_t*)(buf + 4) = htonl(hdr->seq_id);
    *(uint32_t*)(buf + 8) = htonl(hdr->deadline_us);
    *(uint32_t*)(buf + 12) = htonl(hdr->payload_len);
}

void rilp_header_deserialize(const uint8_t* buf, rilp_header_t* hdr) {
    hdr->version = buf[0];
    hdr->src_layer = buf[1];
    hdr->dst_layer = buf[2];
    hdr->msg_type = buf[3];
    hdr->seq_id = ntohl(*(uint32_t*)(buf + 4));
    hdr->deadline_us = ntohl(*(uint32_t*)(buf + 8));
    hdr->payload_len = ntohl(*(uint32_t*)(buf + 12));
}

rilp_message_t* rilp_message_new(uint8_t src, uint8_t dst,
                                  uint8_t msg_type, uint32_t deadline_us,
                                  const uint8_t* payload, uint32_t payload_len) {
    rilp_message_t* msg = malloc(sizeof(rilp_message_t));
    if (!msg) return NULL;
    
    rilp_header_init(&msg->header, src, dst, msg_type, deadline_us);
    msg->header.payload_len = payload_len;
    msg->total_len = RILP_HEADER_SIZE + payload_len;
    
    if (payload_len > 0 && payload) {
        msg->payload = malloc(payload_len);
        if (!msg->payload) {
            free(msg);
            return NULL;
        }
        memcpy(msg->payload, payload, payload_len);
    } else {
        msg->payload = NULL;
    }
    
    return msg;
}

void rilp_message_free(rilp_message_t* msg) {
    if (msg) {
        if (msg->payload) free(msg->payload);
        free(msg);
    }
}

int rilp_send(int sockfd, const rilp_message_t* msg) {
    uint8_t header_buf[RILP_HEADER_SIZE];
    rilp_header_serialize(&msg->header, header_buf);
    
    /* Send header */
    ssize_t sent = send(sockfd, header_buf, RILP_HEADER_SIZE, 0);
    if (sent != RILP_HEADER_SIZE) return -1;
    
    /* Send payload if any */
    if (msg->header.payload_len > 0 && msg->payload) {
        sent = send(sockfd, msg->payload, msg->header.payload_len, 0);
        if (sent != (ssize_t)msg->header.payload_len) return -1;
    }
    
    return 0;
}

rilp_message_t* rilp_recv(int sockfd) {
    uint8_t header_buf[RILP_HEADER_SIZE];
    rilp_message_t* msg;
    rilp_header_t hdr;
    
    /* Receive header */
    ssize_t received = recv(sockfd, header_buf, RILP_HEADER_SIZE, 0);
    if (received != RILP_HEADER_SIZE) return NULL;
    
    /* Parse header */
    rilp_header_deserialize(header_buf, &hdr);
    
    /* Create message with payload */
    uint8_t* payload = NULL;
    if (hdr.payload_len > 0) {
        payload = malloc(hdr.payload_len);
        if (!payload) return NULL;
        
        received = recv(sockfd, payload, hdr.payload_len, 0);
        if (received != (ssize_t)hdr.payload_len) {
            free(payload);
            return NULL;
        }
    }
    
    msg = malloc(sizeof(rilp_message_t));
    if (!msg) {
        if (payload) free(payload);
        return NULL;
    }
    
    memcpy(&msg->header, &hdr, sizeof(rilp_header_t));
    msg->payload = payload;
    msg->total_len = RILP_HEADER_SIZE + hdr.payload_len;
    
    return msg;
}

const char* rilp_msg_type_str(uint8_t msg_type) {
    switch (msg_type) {
        case RILP_MSG_ANNOUNCE:     return "ANNOUNCE";
        case RILP_MSG_HEARTBEAT:    return "HEARTBEAT";
        case RILP_MSG_SENSOR_DATA:  return "SENSOR_DATA";
        case RILP_MSG_ACTUATOR_CMD: return "ACTUATOR_CMD";
        case RILP_MSG_AI_RESULT:    return "AI_RESULT";
        case RILP_MSG_SECURITY_ALERT: return "SECURITY_ALERT";
        case RILP_MSG_OTA_REQUEST:  return "OTA_REQUEST";
        case RILP_MSG_FLEET_STATUS: return "FLEET_STATUS";
        case RILP_MSG_DIAG_QUERY:   return "DIAG_QUERY";
        case RILP_MSG_EMERGENCY:    return "EMERGENCY";
        default: return "UNKNOWN";
    }
}

const char* rilp_layer_str(uint8_t layer) {
    switch (layer) {
        case RILP_LAYER_KERNEL:     return "KERNEL";
        case RILP_LAYER_HAL:        return "HAL";
        case RILP_LAYER_MIDDLEWARE: return "MIDDLEWARE";
        case RILP_LAYER_AI:         return "AI";
        case RILP_LAYER_SECURITY:   return "SECURITY";
        case RILP_LAYER_FLEET:      return "FLEET";
        case RILP_LAYER_BROADCAST:  return "BROADCAST";
        default: return "UNKNOWN";
    }
}
