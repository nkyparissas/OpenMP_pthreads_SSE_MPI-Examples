#!/usr/bin/env bash
sources="source/"
objects="object/"


if [ -d ${objects} ]; 
then
	rm -Rf ${objects}
fi

mkdir ${objects}

for program in "serial" "simd" "simd_mpi"
do
  	if [ $program = "serial" ]; 
  	then
		gcc -o ${objects}${program} -Wall ${sources}${program}.c 
	elif [ $program = "simd" ]  ; 
	then
		gcc -o ${objects}${program} -Wall -msse4.2 ${sources}${program}.c  
	elif [ $program = "simd_mpi" ] ;
	then
		mpicc -o ${objects}${program} -Wall -msse4.2 ${sources}${program}.c -lm 
	fi
done

for N in 100 1000 10000 100000
do
	for program in "serial" "simd"
	do
		echo $program: N = $N
		./${objects}${program} $N 
		echo "------"
	done
	for program in "simd_mpi"
	do 
		for nop in 2 4 8
		do
			echo $program: P = $nop, N = $N
			mpiexec -n $nop ./${objects}${program} $N 	
			echo "------"
		done
	done 
done
