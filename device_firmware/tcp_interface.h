#ifndef TCP_INTERFACE_H
#define TCP_INTERFACE_H

#include <stdint.h>

typedef struct {
    int server_fd;
    int socket_fd;
} TcpConfig;

int tcp_server_init(TcpConfig* server, uint16_t port);

int tcp_send(TcpConfig* server, const uint8_t* data, int data_len);

int tcp_receive(TcpConfig* server, uint8_t* buffer, int buffer_size);

void tcp_close(TcpConfig* server);

#endif // TCP_INTERFACE_H