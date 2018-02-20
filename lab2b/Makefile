# NAME: Atibhav Mittal
# EMAIL: atibhav.mittal6@gmail.com
# ID: 804598987

CC=gcc
CFLAGS=-Wall -Wextra -pthread -g

lab2_list: lab2_list.o SortedList.o
	$(CC) $(CFLAGS) -o $@ -lprofiler lab2_list.o SortedList.o #SortedList.c

lab2_list.o SortedList.o: 
	$(CC) $(CFLAGS) -c lab2_list.c SortedList.c

tests: lab2_list
	rm -f lab2b_list.csv # Remove old copy of the csv file
	touch lab2b_list.csv	# Create the CSV file again

	# Results for lab2b_1.png and lab2b_2.png (only when the sync=m option is specified)
	for num_threads in 1 2 4 8 12 16 24 ; do \
		./lab2_list --iterations=1000 --threads=$$num_threads --sync=s >> lab2b_list.csv ; \
		./lab2_list --iterations=1000 --threads=$$num_threads --sync=m >> lab2b_list.csv ; \
	done

	#echo "TODO"

profile: lab2_list
	-rm -f ./raw.gperf
	CPUPROFILE=./raw.gperf ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text ./lab2_list ./raw.gperf > profile.out
	pprof --list=threadStuff ./lab2_list ./raw.gperf >> profile.out
	rm -f ./raw.gperf

graphs: tests lab2b_list.gp
	./lab2b_list.gp

dist:
	echo "TODO"

clean:
	echo "TODO" 