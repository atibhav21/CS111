# NAME: Atibhav Mittal
# EMAIL: atibhav.mittal6@gmail.com
# ID: 804598987

CC=gcc
CFLAGS=-Wall -Wextra -pthread

build: lab2_add.c #SortedList.h SortedList.c lab2_list.c
	$(CC) $(CFLAGS) -o lab2_add lab2_add.c
	#$(CC) $(CFLAGS) -o lab2_list lab2_add.c SortedList.h SortedList.c

tests: lab2_add #lab2_list

	# Perform tests for each of the synchronization methods with the yield option

	for num_threads in 2 4 8 12 ; do \
		number=1 ; while [[ $$number -le 10 ]] ; do \
			./lab2_add --iterations=1000 --threads=$$num_threads --yield --sync=m ; \
			((number = number + 1)) ; \
		done \
	done



graphs:
	gnuplot lab2_add.gp 
	#gnuplot lab2_list.gp

dist:
	tar -xvzf lab2a-804598987.tar.gz lab2_add.c lab2_list.c SortedList.h SortedList.c lab2_add-?.png lab2_list-?.png lab2_add.csv lab2_list.csv README Makefile