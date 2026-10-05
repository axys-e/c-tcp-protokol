#include "protocol.h"

#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

uint32_t calculate_checksum(Packet *packet){
    uint32_t sum = 0;
    unsigned char *bytes = (unsigned char *)packet;
    for (size_t i = 4; i < sizeof(Packet); i++) {
        sum += bytes[i];
    }
    return sum;
}

int send_all(int sock, const void *buffer, size_t length){
    size_t total = 0;
    while (total < length) {
        ssize_t sent = write(sock, (const char *)buffer + total,
                            length - total);
        if (sent <= 0) {
            return -1;
        }
        total += sent;
    }
    return 0;
}

int receive_all(int sock, void *buffer, size_t length){
    size_t total = 0;
    while (total < length) {
        ssize_t received = read(sock, (char *)buffer + total,
                                length - total);
        if (received <= 0) {
            return -1;
        }
        total += received;
    }
    return 0;
}  

int send_packet(int sock, Packet *packet){
    packet->checksum = calculate_checksum(packet);
    return send_all(sock, packet, sizeof(Packet));
}

int receive_packet(int sock, Packet *packet){
    if (receive_all(sock, packet, sizeof(Packet)) != 0)
{
    perror("receive_all");
    return -1;
}

uint32_t checksum = packet->checksum;

if (checksum != calculate_checksum(packet))
{
    fprintf(stderr, "Checksum error!\n");
    return -2;
}
return 0;
}

