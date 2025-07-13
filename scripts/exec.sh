#!/bin/bash

if [ $1 = 'help' ]
then 
	echo "USAGE: $0 <exe_number> <nodes_number> <tries_number> [EXERCISE OPTIONS...]"
	exit 0
fi

exe_num=$1
nodes=$2
tries=$3
to_exe_args=("${@:4}")
modulo1=$(if [ $exe_num = 1 ]; then echo 3; else echo 2; fi)

if [ -f ./temp.txt ]
then
    rm ./temp.txt
fi

for i in $(seq 1 $tries)
do
    mpiexec -host linux01,linux02 -n $nodes ../build/exe run $exe_num --file ./temp.txt "${to_exe_args[@]}"
    echo "nodes = $nodes" >> ./temp.txt
    if [ $exe_num = 1 ]; then echo "throws = ${to_exe_args[1]}" >> ./temp.txt; else echo "dims = ${to_exe_args[3]}" >> ./temp.txt; fi
done

sum=$(awk "NR % 5 == $modulo1" ./temp.txt | cut -d'=' -f2 | bc | awk '{n += $1}; END{print n}')

avg_time=$(( $sum/$tries ))
avg_pi=''
if [ $exe_num = 1 ]
then 
	sum=$(awk "NR % 5 == 2" ./temp.txt | cut -d'=' -f2 | bc | awk '{n += $1}; END{print n}')
	avg_pi=$(echo "scale=4; $sum/$tries" | bc)
fi 

echo -e "[EXERCISE $exe_num]\ntime = $avg_time" >> ./results.txt
if [ $exe_num = 1 ]  
then 
	echo -e "pi = $avg_pi" >> ./results.txt
fi

echo "nodes = $nodes" >> ./results.txt
if [ $exe_num = 1 ]; then echo "throws = ${to_exe_args[1]}" >> ./results.txt; else echo "dims = ${to_exe_args[3]}" >> ./results.txt; fi

