#!/bin/bash
for i in `seq 1 100`;
do
	mpirun -np $i -hostfile hosts computationtime
done 
