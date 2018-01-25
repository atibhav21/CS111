# NAME: Atibhav Mittal
# EMAIL: atibhav.mittal6@gmail.com
# ID: 804598987

CC=gcc
CFLAGS=-Wall -Wextra
CLIENT_TARGET = lab1b-client
SERVER_TARGET = lab1b-server

default:

client: $(CLIENT_TARGET).c
	$(CC) $(CFLAGS) $(CLIENT_TARGET).c -o $(CLIENT_TARGET)

server: $(SERVER_TARGET).c
	$(CC) $(CFLAGS) $(SERVER_TARGET).c -o $(SERVER_TARGET)

clean:
	rm -rf $(SERVER_TARGET) $(CLIENT_TARGET) lab1b-804598987.tar.gz

dist:
	tar -cvzf lab1b-804598987.tar.gz $(CLIENT_TARGET).c $(SERVER_TARGET).c README Makefile