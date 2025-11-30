#include "tcp_interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>

int tcp_server_init(TcpConfig* server, uint16_t port)
{
    if (!server) return -1;

    // Create server socket
    server->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->server_fd < 0) {
        perror("socket");
        return -1;
    }

    // Allow reuse of address and port
    int opt = 1;
    if (setsockopt(server->server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server->server_fd);
        return -1;
    }

    // Bind socket to port
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server->server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind");
        close(server->server_fd);
        return -1;
    }

    // Listen for incoming connections
    if (listen(server->server_fd, 1) < 0) {
        perror("listen");
        close(server->server_fd);
        return -1;
    }

    printf("Server listening on port %d...\n", port);

    // Accept a single client connection (blocking)
    socklen_t addrlen = sizeof(address);
    server->socket_fd = accept(server->server_fd, (struct sockaddr*)&address, &addrlen);
    if (server->socket_fd < 0) {
        perror("accept");
        close(server->server_fd);
        return -1;
    }

    printf("Client connected.\n");
    return 0;
}

int tcp_send(TcpConfig* server, const uint8_t* data, int data_len)
{
    if (!server || server->socket_fd < 0 || !data || data_len <= 0)
        return -1;

    int total_sent = 0;
    while (total_sent < data_len) {
        int sent = send(server->socket_fd, data + total_sent, data_len - total_sent, 0);
        if (sent < 0) {
            perror("send");
            return -1;
        }
        total_sent += sent;
    }
    return total_sent;
}

int tcp_receive(TcpConfig* server, uint8_t* buffer, int buffer_size)
{
    if (!server || server->socket_fd < 0 || !buffer || buffer_size <= 0)
        return -1;

    int received = recv(server->socket_fd, buffer, buffer_size, 0);
    if (received < 0) {
        perror("recv");
    }
    return received;
}

void tcp_close(TcpConfig* server)
{
    if (!server) return;

    if (server->socket_fd >= 0) {
        close(server->socket_fd);
        server->socket_fd = -1;
    }
    if (server->server_fd >= 0) {
        close(server->server_fd);
        server->server_fd = -1;
    }
}

int generate_tcp_msg(uint8_t* payload_buf, uint16_t payload_buf_size, uint8_t* output_buf, uint16_t output_buf_size)
{
    TcpHeader header;
    static int sample_number = 0;
    header.header_byte_1 = 0x01;
    header.header_byte_2 = 0x02;
    header.sample_number = sample_number;
    sample_number++;

    uint16_t payload_crc = crc16(payload_buf, payload_buf_size);

    if(output_buf_size >= payload_buf_size + sizeof(header) + sizeof(payload_crc))
    {
        memcpy(output_buf, &header, sizeof(header));
        memcpy(output_buf + sizeof(header), payload_buf, payload_buf_size);
        memcpy(output_buf + sizeof(header) + payload_buf_size, &payload_crc, sizeof(payload_crc));

        return 0;
    }
    else 
    {
        return -1;
    }   
}

uint16_t crc16(const uint8_t *data, uint16_t length)
{
    uint16_t crc = 0xFFFF; // Initial value
    for (uint16_t i = 0; i < length; i++)
    {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x0001)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc >>= 1;
        }
    }
    return crc;
}

