# NAME: Atibhav Mittal
# EMAIL: atibhav.mittal6@gmail.com
# ID: 804598987

CC=gcc
CFLAGS=-Wall -Wextra -pthread -g

lab2_list: lab2_list.o SortedList.o
	$(CC) $(CFLAGS) -o $@ -lprofiler lab2_list.o SortedList.o #SortedList.c

lab2_list.o SortedList.o: 
	$(CC) $(CFLAGS) -c lab2_list.c SortedList.c

profile: lab2_list
	-rm -f ./raw.gperf
	CPUPROFILE=./raw.gperf ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text ./lab2_list ./raw.gperf > profile.out
	pprof --list=threadStuff ./lab2_list ./raw.gperf >> profile.out
	rm -f ./raw.gperf

graphs: tests lab2b_list.gp
	./lab2b_list.gp

dist: graphs profile
	tar -cvzf lab2b-804598987.tar.gz SortedList.c SortedList.h lab2_list.c *.png profile.out lab2b_list.csv lab2b_list.gp README Makefile

clean:
	rm -f *.o lab2_list lab2b-804598987.tar.gz

tests: lab2_list
	rm -f lab2b_list.csv # Remove old copy of the csv file
	touch lab2b_list.csv	# Create the CSV file again

	# Results for lab2b_1.png and lab2b_2.png (only when the sync=m option is specified)
	for num_threads in 1 2 4 8 12 16 24 ; do \
		./lab2_list --iterations=1000 --threads=$$num_threads --sync=s >> lab2b_list.csv ; \
		./lab2_list --iterations=1000 --threads=$$num_threads --sync=m >> lab2b_list.csv ; \
	done

	#
	-./lab2_list --yield=id --lists=4 --threads=1 --iterations=1
	-./lab2_list --yield=id --lists=4 --threads=1 --iterations=2
	-./lab2_list --yield=id --lists=4 --threads=1 --iterations=4
	-./lab2_list --yield=id --lists=4 --threads=1 --iterations=8 
	-./lab2_list --yield=id --lists=4 --threads=1 --iterations=16
	-./lab2_list --yield=id --lists=4 --threads=2 --iterations=1
	-./lab2_list --yield=id --lists=4 --threads=2 --iterations=2
	-./lab2_list --yield=id --lists=4 --threads=2 --iterations=4
	-./lab2_list --yield=id --lists=4 --threads=2 --iterations=8 
	-./lab2_list --yield=id --lists=4 --threads=2 --iterations=16
	-./lab2_list --yield=id --lists=4 --threads=4 --iterations=1
	-./lab2_list --yield=id --lists=4 --threads=4 --iterations=2
	-./lab2_list --yield=id --lists=4 --threads=4 --iterations=4
	-./lab2_list --yield=id --lists=4 --threads=4 --iterations=8 
	-./lab2_list --yield=id --lists=4 --threads=4 --iterations=16
	-./lab2_list --yield=id --lists=4 --threads=8 --iterations=1
	-./lab2_list --yield=id --lists=4 --threads=8 --iterations=2
	-./lab2_list --yield=id --lists=4 --threads=8 --iterations=4
	-./lab2_list --yield=id --lists=4 --threads=8 --iterations=8 
	-./lab2_list --yield=id --lists=4 --threads=8 --iterations=16
	-./lab2_list --yield=id --lists=4 --threads=12 --iterations=1
	-./lab2_list --yield=id --lists=4 --threads=12 --iterations=2
	-./lab2_list --yield=id --lists=4 --threads=12 --iterations=4
	-./lab2_list --yield=id --lists=4 --threads=12 --iterations=8 
	-./lab2_list --yield=id --lists=4 --threads=12 --iterations=16
	-./lab2_list --yield=id --lists=4 --threads=16 --iterations=1
	-./lab2_list --yield=id --lists=4 --threads=16 --iterations=2
	-./lab2_list --yield=id --lists=4 --threads=16 --iterations=4
	-./lab2_list --yield=id --lists=4 --threads=16 --iterations=8 
	-./lab2_list --yield=id --lists=4 --threads=16 --iterations=16

	# Results for lab2b_3.png

	# Mutex Tests
	
	./lab2_list --yield=id --lists=4 --threads=1 --iterations=10 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=1 --iterations=20 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=1 --iterations=40 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=1 --iterations=80 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=4 --iterations=10 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=4 --iterations=20 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=4 --iterations=40 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=4 --iterations=80 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=8 --iterations=10 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=8 --iterations=20 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=8 --iterations=40 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=8 --iterations=80 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=12 --iterations=10 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=12 --iterations=20 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=12 --iterations=40 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=12 --iterations=80 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=16 --iterations=10 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=16 --iterations=20 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=16 --iterations=40 --sync=m >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=16 --iterations=80 --sync=m >> lab2b_list.csv

	# Spin Lock Tests
	./lab2_list --yield=id --lists=4 --threads=1 --iterations=10 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=1 --iterations=20 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=1 --iterations=40 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=1 --iterations=80 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=4 --iterations=10 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=4 --iterations=20 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=4 --iterations=40 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=4 --iterations=80 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=8 --iterations=10 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=8 --iterations=20 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=8 --iterations=40 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=8 --iterations=80 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=12 --iterations=10 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=12 --iterations=20 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=12 --iterations=40 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=12 --iterations=80 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=16 --iterations=10 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=16 --iterations=20 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=16 --iterations=40 --sync=s >> lab2b_list.csv
	./lab2_list --yield=id --lists=4 --threads=16 --iterations=80 --sync=s >> lab2b_list.csv


	# lab2b_4.png tests

	./lab2_list --lists=4 --iterations=1000 --threads=1 --sync=m >> lab2b_list.csv
	./lab2_list --lists=4 --iterations=1000 --threads=2 --sync=m >> lab2b_list.csv
	./lab2_list --lists=4 --iterations=1000 --threads=4 --sync=m >> lab2b_list.csv
	./lab2_list --lists=4 --iterations=1000 --threads=8 --sync=m >> lab2b_list.csv
	./lab2_list --lists=4 --iterations=1000 --threads=12 --sync=m	 >> lab2b_list.csv
	./lab2_list --lists=8 --iterations=1000 --threads=1 --sync=m >> lab2b_list.csv
	./lab2_list --lists=8 --iterations=1000 --threads=2 --sync=m >> lab2b_list.csv
	./lab2_list --lists=8 --iterations=1000 --threads=4 --sync=m >> lab2b_list.csv
	./lab2_list --lists=8 --iterations=1000 --threads=8 --sync=m >> lab2b_list.csv
	./lab2_list --lists=8 --iterations=1000 --threads=12 --sync=m >> lab2b_list.csv
	./lab2_list --lists=16 --iterations=1000 --threads=1 --sync=m >> lab2b_list.csv
	./lab2_list --lists=16 --iterations=1000 --threads=2 --sync=m >> lab2b_list.csv
	./lab2_list --lists=16 --iterations=1000 --threads=4 --sync=m >> lab2b_list.csv
	./lab2_list --lists=16 --iterations=1000 --threads=8 --sync=m >> lab2b_list.csv
	./lab2_list --lists=16 --iterations=1000 --threads=12 --sync=m	 >> lab2b_list.csv

	#echo "TODO"
	#lab2b_5.png tests
	./lab2_list --lists=4 --iterations=1000 --threads=1 --sync=s >> lab2b_list.csv
	./lab2_list --lists=4 --iterations=1000 --threads=2 --sync=s >> lab2b_list.csv
	./lab2_list --lists=4 --iterations=1000 --threads=4 --sync=s >> lab2b_list.csv
	./lab2_list --lists=4 --iterations=1000 --threads=8 --sync=s >> lab2b_list.csv
	./lab2_list --lists=4 --iterations=1000 --threads=12 --sync=s	 >> lab2b_list.csv
	./lab2_list --lists=8 --iterations=1000 --threads=1 --sync=s >> lab2b_list.csv
	./lab2_list --lists=8 --iterations=1000 --threads=2 --sync=s >> lab2b_list.csv
	./lab2_list --lists=8 --iterations=1000 --threads=4 --sync=s >> lab2b_list.csv
	./lab2_list --lists=8 --iterations=1000 --threads=8 --sync=s >> lab2b_list.csv
	./lab2_list --lists=8 --iterations=1000 --threads=12 --sync=s >> lab2b_list.csv
	./lab2_list --lists=16 --iterations=1000 --threads=1 --sync=s >> lab2b_list.csv
	./lab2_list --lists=16 --iterations=1000 --threads=2 --sync=s >> lab2b_list.csv
	./lab2_list --lists=16 --iterations=1000 --threads=4 --sync=s >> lab2b_list.csv
	./lab2_list --lists=16 --iterations=1000 --threads=8 --sync=s >> lab2b_list.csv
	./lab2_list --lists=16 --iterations=1000 --threads=12 --sync=s	 >> lab2b_list.csv
