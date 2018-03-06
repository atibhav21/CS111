# NAME: Atibhav Mittal
# EMAIL: atibhav.mittal6@gmail.com
# ID: 804598987

CC=gcc
CFLAGS= -Wall -Wextra

TARGET=lab3a

$(TARGET): $(TARGET).c 
	$(CC) $(CFLAGS) $(TARGET).c -o $(TARGET)

dist: $(TARGET).c Makefile README ext2_fs.h
	tar -cvzf lab3a-804598987.tar.gz $^

clean:
	rm -f lab3a-804598987.tar.gz $(TARGET)