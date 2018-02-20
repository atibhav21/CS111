#! /usr/bin/gnuplot
#
# purpose:
#	 generate data reduction graphs for the multi-threaded list project for lab2b
#
# input: lab2b_list.csv
#	1. test name
#	2. # threads
#	3. # iterations per thread
#	4. # lists
#	5. # operations performed (threads x iterations x (ins + lookup + delete))
#	6. run time (ns)
#	7. run time per operation (ns)
#
# output:
#	lab2_list-1.png ... Throughput vs number of threads graph (For spin lock and mutex)
#	lab2_list-2.png ... 
#	lab2_list-3.png ... 
#	lab2_list-4.png ... 
#
# Note:
#	Managing data is simplified by keeping all of the results in a single
#	file.  But this means that the individual graphing commands have to
#	grep to select only the data they want.
#
#	Early in your implementation, you will not have data for all of the
#	tests, and the later sections may generate errors for missing data.
#

# general plot parameters
set terminal png
set datafile separator ","

# how many threads/iterations we can run without failure (w/o yielding)
set title "List-1: Throughput vs Threads"
set xlabel "Threads"
set ylabel "Throughput (/s)"
set logscale y 10
set output 'lab2b_1.png'

# grep out only synchronized mutex/spin lock
plot \
     "< grep 'list-none-s,' lab2b_list.csv" using ($2):(10000000/$7) \
	title 'Spin Lock' with linespoints lc rgb 'red', \
     "< grep 'list-none-m,' lab2b_list.csv" using ($2):(10000000/$7) \
	title 'Mutex' with linespoints lc rgb 'green'

set title "List-2: Mutex Wait Time and Average Time/ op vs Num of Threads"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Time (ns)"
set logscale y 10
set output 'lab2b_2.png'
# note that unsuccessful runs should have produced no output
plot \
     "< grep list-none-m lab2b_list.csv" using ($2):($8) \
	title 'Mutex Wait Time' with linespoints lc rgb 'green', \
     "< grep list-none-m lab2b_list.csv" using ($2):($7) \
	title 'Average Time per op' with linespoints lc rgb 'red'
     