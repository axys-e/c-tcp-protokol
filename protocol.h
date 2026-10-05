#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <stddef.h>

#define DATA_SIZE 1024




typedef struct __attribute__((packed)) {
    uint32_t checksum;
    char type[4];
    uint32_t packetNumber;
    uint32_t dataLength;
    unsigned char data[DATA_SIZE];
} Packet;

void print_packet(const char *richtung, Packet *packet);

#define TYPE_ECHO_REQUEST "\xF0\x9F\x93\xA3"
#define TYPE_ECHO_REPLY "\xF0\x9F\x93\xA4"

#define TYPE_DATE_REQUEST "\xF0\x9F\x93\x85"
#define TYPE_DATE_REPLY_LE "\xF0\x9F\x93\x84"
#define TYPE_DATE_REPLY_BE "\xF0\x9F\x93\x86"

#define TYPE_FILE_REQUEST "\xF0\x9F\x93\x81"
#define TYPE_FILE_DATA "\xF0\x9F\x93\x82"
#define TYPE_STATISTICS "\xF0\x9F\x93\x8A"

#define TYPE_OK "\xF0\x9F\x91\x8D"
#define TYPE_NOPE "\xF0\x9F\x91\x8E"

#define TYPE_ERROR "\xF0\x9F\x92\xA9"
#define TYPE_HEARTBEAT "\xF0\x9F\x92\x93"

uint32_t calculate_checksum(Packet *packet);

int send_all(int sock, const void *buffer, size_t length);
int receive_all(int sock, void *buffer, size_t length);

int send_packet(int sock, Packet *packet);
int receive_packet(int sock, Packet *packet);



#endif

