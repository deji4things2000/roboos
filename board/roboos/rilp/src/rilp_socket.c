#include "rilp.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int rilp_create_server(const char* socket_path) {
    int sockfd;
    struct sockaddr_un addr;
    
    /* Remove existing socket file */
    unlink(socket_path);
    
    /* Create socket */
    sockfd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (sockfd < 0) return -1;
    
    /* Bind to address */
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);
    
    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sockfd);
        return -1;
    }
    
    /* Listen for connections */
    if (listen(sockfd, 5) < 0) {
        close(sockfd);
        return -1;
    }
    
    return sockfd;
}

int rilp_accept_connection(int server_sock) {
    int client_sock;
    struct sockaddr_un client_addr;
    socklen_t addr_len = sizeof(client_addr);
    
    client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_len);
    if (client_sock < 0) return -1;
    
    return client_sock;
}

int rilp_connect(const char* socket_path) {
    int sockfd;
    struct sockaddr_un addr;
    
    sockfd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (sockfd < 0) return -1;
    
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);
    
    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sockfd);
        return -1;
    }
    
    return sockfd;
}

void rilp_close(int sockfd) {
    if (sockfd >= 0) close(sockfd);
}
