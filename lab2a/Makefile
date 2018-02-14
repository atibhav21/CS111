# NAME: Atibhav Mittal
# EMAIL: atibhav.mittal6@gmail.com
# ID: 804598987

CC=gcc
#TODO: REMOVE THE -g
CFLAGS=-Wall -Wextra -pthread -g 

build: lab2_add.c #SortedList.h SortedList.c lab2_list.c
	$(CC) $(CFLAGS) -o lab2_add lab2_add.c
	$(CC) $(CFLAGS) -o lab2_list lab2_list.c SortedList.h SortedList.c

tests: lab2_add #lab2_list
	rm -f lab2_add.csv # Delete the old file if it exists
	touch lab2_add.csv # Create a new lab2_add.csv file

	chmod +x test_script.sh # TODO: Check if needed
	./test_script.sh

graphs:
	./lab2_add.gp 
	#gnuplot lab2_list.gp

dist:
	tar -xvzf lab2a-804598987.tar.gz lab2_add.c lab2_list.c SortedList.h SortedList.c lab2_add-?.png lab2_list-?.png lab2_add.csv lab2_list.csv README Makefile