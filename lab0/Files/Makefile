# NAME: Atibhav Mittal
# EMAIL: atibhav.mittal6@gmail.com
# ID: 804598987

CC=gcc
CFLAGS=-Wall -Wextra
TARGET=lab0

$(TARGET): $(TARGET).c 
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

test1: $(TARGET)
	echo "Hello, this is a sample text file for a smoke test This file checks if the text from one file is successfully copied to another file!" > abc.txt
	./$(TARGET) --input=abc.txt --output=def.txt
	diff abc.txt def.txt
	rm -f abc.txt def.txt

test2: $(TARGET)
	cat <<< "Hello There" | ./$(TARGET) --output=test2output.txt 
	echo "Hello There" > correct2output.txt
	diff correct2output.txt test2output.txt
	rm -f test2output.txt correct2output.txt

test3: $(TARGET)
	echo "Some random text for some random test case" > test3input.txt
	./$(TARGET) --input=test3input.txt > test3output.txt
	diff test3input.txt test3output.txt
	rm test3input.txt test3output.txt

test4: $(TARGET)
	-./$(TARGET) --input=nosuchfile.txt; test $$? -eq 2

test5: $(TARGET)
	-./$(TARGET) --segfault
	-./$(TARGET) --segfault --catch ; test $$? -eq 4

check: test1 test2 test3 test4 test5
	echo "Successfully finished smoke tests!"

clean:
	rm -f lab0 lab0-804598987.tar.gz 

dist:
	tar -cvzf lab0-804598987.tar.gz README lab0.c Makefile backtrace.png breakpoint.png




