#!/usr/bin/env bash
sources="source/"
objects="object/"


if [ -d ${objects} ]; 
then
	rm -Rf ${objects}
fi

mkdir ${objects}

for program in "cereal" "openmp1" "openmp2" "openmp3" "pthread1" "pthread2" "pthread3"
do
  	if [ $program = "cereal" ]; 
  	then
		gcc -o ${objects}${program} -Wall ${sources}${program}.c
	elif [ $program = "openmp1" -o $program = "openmp2" -o $program = "openmp3" ]  ; 
	then
		gcc -o ${objects}${program} -Wall -fopenmp ${sources}${program}.c -lm
	elif [ $program = "pthread1" -o $program = "pthread2" -o $program = "pthread3" ] ;
	then
		gcc -o ${objects}${program} -Wall -pthread ${sources}${program}.c -lm
	fi
done

for program in "cereal" "openmp1" "openmp2" "openmp3" "pthread1" "pthread2" "pthread3"
do
	if [ "$#" != 0 ]; 
	then 
		./${objects}${program} ${1} ${2} ${3} ${4} ${5}	
	fi	
done