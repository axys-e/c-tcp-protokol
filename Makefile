CC = gcc
CFLAGS = -Wall -Wextra -pedantic

TARGET = tcp_client
SRC = tcp_client.c protocol.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)