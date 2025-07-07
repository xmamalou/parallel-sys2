#!/bin/bash

exe_num=$1
nodes=$2
tries=$3
to_exe_args=("${@:4}")
modulo=$(if [ $exe_num = 1 ]; then echo 0; else echo 2; fi)


echo "${to_exe_args[@]}"
if [ -f ./temp.txt ]
then
    rm ./temp.txt
fi

for i in $(seq 1 $tries)
do
    mpiexec -n $nodes ../build/exe run $exe_num --file ./temp.txt "${to_exe_args[@]}"
done

sum=$(awk "NR % 3 == $modulo" ./temp.txt | cut -d'=' -f2 | bc | awk '{n += $1}; END{print n}')

echo $(( $sum/$tries ))
