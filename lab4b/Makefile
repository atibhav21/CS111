# NAME: Atibhav Mittal
# EMAIL: atibhav.mittal6@gmail.com
# ID: 804598987

CC=gcc
CFLAGS=-Wall -Wextra -lmraa -lm

TARGET=lab4b

$(TARGET): $(TARGET).c 
	$(CC) $(CFLAGS) $(TARGET).c -o $(TARGET)

check: $(TARGET)

	echo "OFF" | ./lab4b 
	echo "OFF" | ./lab4b --scale=C 
	echo "OFF" | ./lab4b --period=10
	echo "OFF" | ./lab4b --scale=F --log=abc.txt
	rm abc.txt

clean:
	rm -f $(TARGET) lab4b-804598987.tar.gz

dist:
	tar -cvzf lab4b-804598987.tar.gz $(TARGET).c Makefile README