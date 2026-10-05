#include "protocol.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <time.h>
#include <fcntl.h>

#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER_IP "ip"
#define SERVER_PORT port

int echo_command(int sock, const char *message, int verbose) {
    size_t length = strlen(message);
    if (length > DATA_SIZE) {
        printf("Nachricht zu lang. Maximale Länge: %d\n", DATA_SIZE);
        return -1;
    }
    Packet packet;
    memset(&packet, 0, sizeof(Packet));
    memcpy(packet.type, TYPE_ECHO_REQUEST, 4);
    packet.packetNumber = 1;
    packet.dataLength = (uint32_t)length;
    memcpy(packet.data, message, packet.dataLength);
    if (send_packet(sock, &packet) != 0) {
        return -1;
    }

    if (verbose) {
        print_packet("SEND", &packet);
    }
    Packet answer;
    if (receive_packet(sock, &answer) != 0) {
        return -1;
    }
    if (verbose) {
        print_packet("RECV", &answer);
    }
    if (memcmp(answer.type, TYPE_ECHO_REPLY, 4) != 0) {
        printf("Falscher Antworttyp\n");
        return -1;
    }
    printf("Antwort: %.*s\n", answer.dataLength, answer.data);
    return 0;
}


uint64_t swap_uint64(uint64_t value)
{
    uint64_t result = 0;

    for (int i = 0; i < 8; i++)
    {
        result <<= 8;
        result |= value & 0xFF;
        value >>= 8;
    }

    return result;
}

int date_command(int sock, int verbose) {
    Packet packet;
    memset(&packet, 0, sizeof(Packet));
    memcpy(packet.type, TYPE_DATE_REQUEST, 4);
    packet.packetNumber = 1;
    packet.dataLength = 0;
    if (send_packet(sock, &packet) != 0) {
        return -1;
    }

    if (verbose) {
        print_packet("SEND", &packet);
    }

    Packet answer;
    if (receive_packet(sock, &answer) != 0) {
        return -1;
    }
    if (verbose) {
        print_packet("RECV", &answer);
    }
    if (memcmp(answer.type, TYPE_DATE_REPLY_LE, 4) == 0){
        printf("Little Endian Antwort\n");
    }
    else if (memcmp(answer.type, TYPE_DATE_REPLY_BE, 4) == 0){
        printf("Big Endian Antwort\n");
    }
    else{
        printf("Falscher Antworttyp\n");
        return -1;
    }
    if (answer.dataLength != 4 && answer.dataLength != 8) {
        printf("Falsche Datenlänge: %u\n", answer.dataLength);
        return -1;
    }

    
    time_t timestamp;
    

    if (answer.dataLength == 4) {
        uint32_t value;
        memcpy(&value, answer.data, sizeof(value));
        timestamp = (time_t)value;
    } else {
        uint64_t value;
        memcpy(&value, answer.data, sizeof(value));
        if (memcmp(answer.type, TYPE_DATE_REPLY_BE, 4) == 0) {
            value = swap_uint64(value);
        }

        timestamp = (time_t)value;
    }

    struct tm *timeinfo = localtime(&timestamp);
    if (timeinfo == NULL) {
        perror("localtime");
        return -1;
    }

    char buffer[64];

    strftime(buffer, sizeof(buffer), "%d.%m.%Y %H:%M:%S", timeinfo);

    printf("%s\n", buffer);

    return 0;
}

int send_ok(int sock, uint32_t packetNumber)
{
    Packet packet;

    memset(&packet, 0, sizeof(Packet));

    memcpy(packet.type, TYPE_OK, 4);

    packet.packetNumber = packetNumber;
    packet.dataLength = 0;

    return send_packet(sock, &packet);
}

int send_nope(int sock, uint32_t packetNumber)
{
    Packet packet;

    memset(&packet, 0, sizeof(Packet));

    memcpy(packet.type, TYPE_NOPE, 4);

    packet.packetNumber = packetNumber;
    packet.dataLength = 0;

    return send_packet(sock, &packet);
}

void print_packet(const char *richtung, Packet *packet)
{
    printf("[%s]\n", richtung);

    printf("Typ: %.4s\n", packet->type);
    printf("Packetnummer: %u\n", packet->packetNumber);
    printf("DataLength: %u\n", packet->dataLength);
    printf("Checksum: %u\n", packet->checksum);

    printf("\n");
}

int file_command(int sock, const char *remoteFile, const char *localFile, int verbose) {
    Packet packet;
    memset(&packet, 0, sizeof(Packet));
    memcpy(packet.type, TYPE_FILE_REQUEST, 4);
    packet.packetNumber = 1;
    size_t length = strlen(remoteFile);
    if (length > DATA_SIZE) {
        printf("Dateiname zu lang. Maximale Länge: %d\n", DATA_SIZE);
        return -1;
    }
    packet.dataLength = (uint32_t)length;
    memcpy(packet.data, remoteFile, packet.dataLength);
    if (send_packet(sock, &packet) != 0) {
        return -1;
    }

    if (verbose) {
        print_packet("SEND", &packet);
    }

    Packet answer;
    if (receive_packet(sock, &answer) != 0) {
        return -1;
    }
    if (verbose) {
        print_packet("RECV", &answer);
    }
    if (memcmp(answer.type, TYPE_STATISTICS, 4) != 0) {
        printf("STATISTICS erwartet\n");
        return -1;
    }

    int file_fd = open(localFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (file_fd == -1)
    {
        perror("open");
        return -1;
    }

    if (answer.dataLength != 8) {
        printf("Ungueltige Statistikpaket\n");
        close(file_fd);
        return -1;
    }

    uint32_t fileSize;
    uint32_t fileChecksum;
    uint32_t bytesReceived = 0;
    uint32_t clientPacketNumber = 2;

    memcpy(&fileSize, answer.data, sizeof(fileSize));
    memcpy(&fileChecksum, answer.data + sizeof(fileSize), sizeof(fileChecksum));
    printf("Dateigröße: %u Byte\n", fileSize);
    printf("Datei-Checksum: %u\n", fileChecksum);

    if (send_ok(sock, clientPacketNumber) != 0) {
        close(file_fd);
        return -1;
    }
    clientPacketNumber++;

    if (verbose){
        Packet okPacket;

        memset(&okPacket, 0, sizeof(Packet));

        memcpy(okPacket.type, TYPE_OK, 4);
        okPacket.packetNumber = clientPacketNumber - 1;
        okPacket.dataLength = 0;
        okPacket.checksum = calculate_checksum(&okPacket);

        print_packet("SEND", &okPacket);
    }
    
    uint32_t expectedServerPacket = 2;

    

    while (bytesReceived < fileSize){
        int result = receive_packet(sock, &answer);

        if (result == -1)
            {
            close(file_fd);
            return -1;
            }

        if (result == -2)
        {
            
            printf("Paket beschaedigt. Sende NOPE.\n");
            if (send_nope(sock, clientPacketNumber) != 0)
            {
            close(file_fd);
            return -1;
            }
        
        if (verbose)
            {
            Packet nopePacket;

            memset(&nopePacket, 0, sizeof(Packet));

            memcpy(nopePacket.type, TYPE_NOPE, 4);
            nopePacket.packetNumber = clientPacketNumber;
            nopePacket.dataLength = 0;
            nopePacket.checksum = calculate_checksum(&nopePacket);

            print_packet("SEND", &nopePacket);
            }

        clientPacketNumber++;
        expectedServerPacket++;
        continue;
        }

        if (verbose) {
            print_packet("RECV", &answer);
        }

        if (memcmp(answer.type, TYPE_FILE_DATA, 4) == 0) {

            if (answer.packetNumber != expectedServerPacket)
            {
                printf("Falsche Paketnummer. Erwartet: %u, erhalten: %u\n", expectedServerPacket,
                answer.packetNumber);

                close(file_fd);
                return -1;
            }

            expectedServerPacket++;

            if (answer.dataLength > DATA_SIZE) {
                printf("Ungueltige Paketgrosse\n");
                close(file_fd);
                return -1;
            }
            
            ssize_t written = write(file_fd, answer.data, answer.dataLength);
            if (written == -1) {
                perror("write");
                close(file_fd);
                return -1;
            }

            if ((uint32_t)written != answer.dataLength) {
                printf("Nicht alle Bytes geschrieben\n");
                close(file_fd);
                return -1;
            }

            bytesReceived += answer.dataLength;

            printf("Empfangen: %u/%u Bytes\n", bytesReceived, fileSize);

            if (bytesReceived == fileSize){
                break;
            }

            if (send_ok(sock, clientPacketNumber) != 0) {
                close(file_fd);
                return -1;
            }

            if (verbose){
                Packet okPacket;

                memset(&okPacket, 0, sizeof(Packet));

                memcpy(okPacket.type, TYPE_OK, 4);
                okPacket.packetNumber = clientPacketNumber;
                okPacket.dataLength = 0;
                okPacket.checksum = calculate_checksum(&okPacket);

                print_packet("SEND", &okPacket);
            }

            clientPacketNumber++;

            
        }
        else if (memcmp(answer.type, TYPE_HEARTBEAT, 4) == 0) {
            
            if (answer.packetNumber != expectedServerPacket)
            {
                printf("Falsche Paketnummer. Erwartet: %u, erhalten: %u\n", expectedServerPacket,
                answer.packetNumber);

                close(file_fd);
                return -1;
            }

            expectedServerPacket++;


            if (send_ok(sock, clientPacketNumber) != 0) {
                close(file_fd);
                return -1;
            }
            if (verbose){
                Packet okPacket;

                memset(&okPacket, 0, sizeof(Packet));

                memcpy(okPacket.type, TYPE_OK, 4);
                okPacket.packetNumber = clientPacketNumber;
                okPacket.dataLength = 0;
                okPacket.checksum = calculate_checksum(&okPacket);

                print_packet("SEND", &okPacket);
            }

            clientPacketNumber++;
            continue;
        }
        else if (memcmp(answer.type, TYPE_ERROR, 4) == 0) {
            printf("Serverfehler: %.*s\n", answer.dataLength, answer.data);
            close(file_fd);
            return -1;
        }
        else {
            printf("Ungueltiger Pakettyp empfangen\n");
            close(file_fd);
            return -1;
        }

    }
    printf("Datei erfolgreich empfangen.\n");
    close(file_fd);
    file_fd = open(localFile, O_RDONLY);
    if (file_fd == -1) {
        perror("open");
        return -1;
    }
    unsigned char buffer[1024];
    uint32_t checksum = 0;
    ssize_t n;

    while ((n = read(file_fd, buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < n; i++) {
            checksum += buffer[i];
        }
    }
    if (n == -1) {
        perror("read");
        close(file_fd);
        return -1;
    }

    if (checksum == fileChecksum) {
        printf("Checksum korrekt\n");

        if (send_ok(sock, clientPacketNumber) != 0)
            {
            close(file_fd);
            return -1;
            }

        if (verbose)
            {
            Packet okPacket;

            memset(&okPacket, 0, sizeof(Packet));

            memcpy(okPacket.type, TYPE_OK, 4);
            okPacket.packetNumber = clientPacketNumber;
            okPacket.dataLength = 0;
            okPacket.checksum = calculate_checksum(&okPacket);

            print_packet("SEND", &okPacket);
            }
    } else {
        printf("Checksum fehler\n");

        if (send_nope(sock, clientPacketNumber) != 0)
        {
            close(file_fd);
            return -1;
        }

        if (verbose)
        {
            Packet nopePacket;

            memset(&nopePacket, 0, sizeof(Packet));

            memcpy(nopePacket.type, TYPE_NOPE, 4);
            nopePacket.packetNumber = clientPacketNumber;
            nopePacket.dataLength = 0;
            nopePacket.checksum = calculate_checksum(&nopePacket);

            print_packet("SEND", &nopePacket);
        }
    }

    close(file_fd);

    return 0;
}

int main(int argc, char const *argv[])
{

    

    if (argc < 2)
    {
        printf("Benutzung:\n");
        printf("./tcp_client echo <nachricht>\n");
        printf("./tcp_client date\n");
        printf("./tcp_client file <remote> <lokal>\n");
        return 1;
    }

    int verbose = 0;
    int first = 1;

        if(strcmp(argv[1], "-v") == 0) {
        verbose = 1;
        first = 2;
    }

    if(strcmp(argv[argc - 1], "-v") == 0) {
        verbose = 1;
    }


    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock);
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) == -1) {
        perror("connect");
        close(sock);
        return 1;
    }

    printf("Verbindung erfolgreich hergestellt!\n");




    if (strcmp(argv[first], "echo") == 0){
        if(first + 1 >= argc){
            printf("Fehlende Nachricht\n");
            close(sock);
            return 1;
        }
        if (echo_command(sock, argv[first + 1], verbose) != 0) {
            printf("Fehler beim Senden der Echo-Nachricht\n");
            close(sock);
            return 1;
        }
    }

    else if (strcmp(argv[first], "date") == 0){
        if (date_command(sock, verbose) != 0)
        {
            printf("Fehler beim Abrufen des Datums\n");
            close(sock);
            return 1;
        }
    }

    else if (strcmp(argv[first], "file") == 0){
        if (first + 2 >= argc)
        {
            printf("Benutzung: ./tcp_client file <remote> <lokal>\n");
            close(sock);
            return 1;
        }

        if (file_command(sock, argv[first + 1], argv[first + 2], verbose) != 0)
        {
            printf("Dateiübertragung fehlgeschlagen\n");
            close(sock);
            return 1;
        }
    }

    else{
        printf("Unbekannter Befehl: %s\n", argv[first]);
        close(sock);
        return 1;
    }

    

    
    close(sock);
    return 0;
}

