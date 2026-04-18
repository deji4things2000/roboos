#include "rilp.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#define SOCKET_PATH "/tmp/rilp_test.sock"

void* server_thread(void* arg) {
    (void)arg; /* unused */
    int server_sock = rilp_create_server(SOCKET_PATH);
    if (server_sock < 0) {
        perror("Server: create failed");
        return NULL;
    }
    
    printf("Server waiting for connection on %s\n", SOCKET_PATH);
    int client_sock = rilp_accept_connection(server_sock);
    if (client_sock < 0) {
        perror("Server: accept failed");
        close(server_sock);
        return NULL;
    }
    
    printf("Server: client connected\n");
    
    /* Receive message */
    rilp_message_t* msg = rilp_recv(client_sock);
    if (msg) {
        printf("Server received:\n");
        printf("  src_layer: %s (%d)\n", 
               rilp_layer_str(msg->header.src_layer), msg->header.src_layer);
        printf("  dst_layer: %s (%d)\n", 
               rilp_layer_str(msg->header.dst_layer), msg->header.dst_layer);
        printf("  msg_type: %s (0x%02X)\n", 
               rilp_msg_type_str(msg->header.msg_type), msg->header.msg_type);
        printf("  seq_id: %u\n", msg->header.seq_id);
        printf("  deadline_us: %u\n", msg->header.deadline_us);
        printf("  payload_len: %u\n", msg->header.payload_len);
        
        if (msg->header.payload_len > 0 && msg->payload) {
            printf("  payload: %.*s\n", (int)msg->header.payload_len, (char*)msg->payload);
        }
        
        rilp_message_free(msg);
    }
    
    rilp_close(client_sock);
    rilp_close(server_sock);
    unlink(SOCKET_PATH);
    
    return NULL;
}

void client_send_test_message(void) {
    int sock = rilp_connect(SOCKET_PATH);
    if (sock < 0) {
        perror("Client: connect failed");
        return;
    }
    
    /* Test payload */
    const char* test_payload = "Hello from RoboOS RILP!";
    
    /* Create RILP message from HAL to Middleware with 1ms deadline */
    rilp_message_t* msg = rilp_message_new(
        RILP_LAYER_HAL,          /* src: HAL */
        RILP_LAYER_MIDDLEWARE,   /* dst: Middleware */
        RILP_MSG_SENSOR_DATA,    /* type: sensor data */
        1000,                    /* deadline: 1ms */
        (const uint8_t*)test_payload,
        strlen(test_payload) + 1
    );
    
    if (msg) {
        printf("Client sending RILP message...\n");
        rilp_send(sock, msg);
        rilp_message_free(msg);
        printf("Client: message sent\n");
    }
    
    rilp_close(sock);
}

int main(void) {
    pthread_t server_tid;
    
    printf("=== RILP Protocol Test ===\n");
    printf("Version: %d\n", RILP_VERSION);
    printf("Header size: %d bytes\n", RILP_HEADER_SIZE);
    printf("\n");
    
    /* Start server */
    pthread_create(&server_tid, NULL, server_thread, NULL);
    
    /* Wait for server to be ready */
    usleep(100000);
    
    /* Client sends message */
    client_send_test_message();
    
    /* Wait for server to finish */
    pthread_join(server_tid, NULL);
    
    printf("\n=== RILP Test Complete ===\n");
    return 0;
}
