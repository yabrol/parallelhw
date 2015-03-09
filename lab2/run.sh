mpicc -c old_sample.c -o sample.o
mpicc -c api.c -o api.o
mpicc -g sample.o api.o -o sample
#./sample
mpirun -np 5 -hostfile hosts sample
