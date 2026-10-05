TCP Client in C

A C-based TCP client for communicating with a server using a custom packet-based protocol.

The project was developed as a networking/programming assignment and demonstrates low-level TCP socket communication, custom packet handling, checksum verification, file transfer, date/time conversion, and verbose protocol logging.

Features

TCP connection using IPv4 sockets

Custom packet-based communication

Echo requests and replies

Server date/time requests

Remote file download

Packet sequence number validation

Packet checksum handling

Detection of corrupted packets with OK / NOPE responses

Heartbeat packet handling during file transfer

Support for both 32-bit and 64-bit timestamps

Little-endian / big-endian date responses

Verbose packet logging with -v

Basic error handling for invalid packets and failed socket/file operations

Available commands

Echo

Sends a message to the server and prints the response.

./tcp_client echo "Hello World"

Verbose mode:

./tcp_client -v echo "Hello World"

The client creates an ECHO request packet and expects an ECHO reply.

Date

Requests the current date/time from the server.

./tcp_client date

Verbose mode:

./tcp_client -v date

The client handles both 4-byte and 8-byte timestamp values and converts the received timestamp into local date/time format.

Example output:

06.10.2026 00:30:15

File transfer

Downloads a file from the server and saves it under a local filename.

./tcp_client file <remote-file> <local-file>

Example:

./tcp_client file test.txt downloaded.txt

Verbose mode:

./tcp_client -v file test.txt downloaded.txt

During the transfer, the client:

Requests the remote file.

Receives file size and the expected file checksum.

Confirms the statistics packet.

Receives the file in multiple packets.

Checks packet numbers.

Handles heartbeat packets.

Responds with OK or NOPE when required.

Calculates the checksum of the downloaded file.

Compares the calculated checksum with the checksum supplied by the server.

Verbose mode

The -v option prints information about sent and received packets.

Example:

[SEND]
Typ: ...
Packetnummer: 1
DataLength: ...
Checksum: ...

[RECV]
Typ: ...
Packetnummer: ...
DataLength: ...
Checksum: ...

This is useful for debugging the communication protocol and checking packet sequence numbers and checksums.

Protocol overview

The client communicates using a custom Packet structure defined in the project's protocol header.

Packets contain fields such as:

packet type

packet number

data length

data

checksum

The client uses different packet types for different operations, including:

echo request / reply

date request / reply

file request

file data

statistics

heartbeat

OK

NOPE

error packets

The implementation also validates packet types, packet numbers, data lengths, and checksums where required.

Project structure

A typical project layout is:

.
├── tcp_client.c
├── protocol.h
├── protocol.c
└── README.md

tcp_client.c contains the client-side command handling and TCP communication logic.

protocol.h contains the packet structure and protocol definitions.

protocol.c contains shared packet operations such as sending, receiving, and checksum calculation.

Requirements

The program requires:

Linux / Unix-like operating system

C compiler such as GCC

POSIX socket support

The source uses POSIX networking and file APIs including:

socket()
connect()
send()
recv()
open()
read()
write()
close()

Compilation

If the project contains tcp_client.c and protocol.c, it can typically be compiled with:

gcc -Wall -Wextra -o tcp_client tcp_client.c protocol.c

For debugging symbols:

gcc -Wall -Wextra -g -o tcp_client tcp_client.c protocol.c

The exact compilation command may depend on the accompanying protocol implementation.

Server configuration

The server address and port are configured in the source code:

#define SERVER_IP "ip"
#define SERVER_PORT port

Replace these values with the address and port of the target server before compiling.

Error handling

The client checks for common communication and file errors, including:

invalid server address

failed socket creation

failed TCP connection

invalid packet types

unexpected packet numbers

invalid data lengths

corrupted packets

failed file creation

failed file reads/writes

checksum mismatches

If a received packet is detected as corrupted, the client can respond with a NOPE packet.

Learning goals

This project demonstrates practical concepts in:

TCP/IP networking

POSIX sockets

client-server communication

binary packet protocols

network byte order and endianness

checksums

file I/O

error handling

command-line argument parsing

debugging network protocols

Disclaimer

This project was created for educational purposes. It is intended to demonstrate TCP networking and custom protocol implementation in C.