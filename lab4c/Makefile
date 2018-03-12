# NAME: Atibhav Mittal
# EMAIL: atibhav.mittal6@gmail.com
# ID: 804598987

CC=gcc
CFLAGS=-Wall -Wextra -lmraa -lm

TARGET=lab4c_tcp
TARGET2=lab4c_tls

lab4c: $(TARGET).c $(TARGET2).c
	$(CC) $(CFLAGS) $(TARGET).c -o $(TARGET)
	$(CC) $(CFLAGS) -lssl -lcrypto $(TARGET2).c -o $(TARGET2)

clean:
	rm -f $(TARGET) $(TARGET2) lab4c-804598987.tar.gz

dist:
	tar -cvzf lab4c-804598987.tar.gz $(TARGET).c $(TARGET2).c Makefile README