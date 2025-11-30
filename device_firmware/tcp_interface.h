#ifndef TCP_INTERFACE_H
#define TCP_INTERFACE_H

#include <stdint.h>


typedef struct {
    int server_fd;
    int socket_fd;
} TcpConfig;

typedef struct __attribute__((packed)){
    uint8_t header_byte_1;
    uint8_t header_byte_2;
    uint32_t sample_number;
} TcpHeader;

int tcp_server_init(TcpConfig* server, uint16_t port);

int tcp_send(TcpConfig* server, const uint8_t* data, int data_len);

int tcp_receive(TcpConfig* server, uint8_t* buffer, int buffer_size);

void tcp_close(TcpConfig* server);

int generate_tcp_msg(uint8_t* payload_buf, uint16_t payload_buf_size, uint8_t* output_buf, uint16_t output_buf_size);
uint16_t crc16(const uint8_t *data, uint16_t length);

#endif // TCP_INTERFACE_H