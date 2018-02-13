tests: 

		# Perform tests for add-none

	for num_threads in 2 4 8 12 ; do \
		for num_iterations in 100 1000 10000 100000 ; do \
			./lab2_add  --iterations=$$num_iterations --threads=$$num_threads ; \
			((repeat = repeat + 1)) ; \
		done \
	done 

	# Perform tests for add-yield-none

	for num_threads in 2 4 8 12 ; do \
		for num_iterations in 10 20 40 80 100 1000 10000 100000 ; do \
			number=1 ; while [[ $$number -le 10 ]] ; do \
				./lab2_add --iterations=$$num_iterations --threads=$$num_threads --yield ; \
				((number = number + 1)) ; \
			done \
		done \
	done

	# Perform tests for each of the synchronization methods with the yield option

	for num_threads in 2 4 8 12 ; do \
		number=1 ; while [[ $$number -le 10 ]] ; do \
			#./lab2_add --iterations=10000 --threads=$$num_threads --yield --sync=m ; 
			./lab2_add --iterations=1000 --threads=$$num_threads --yield --sync=c ; \
			((number = number + 1)) ; \
		done
	done