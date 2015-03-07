mpicc -c sample.c -o sample.o
mpicc -c mw_api.c -o mw_api.o
mpicc sample.o mw_api.o -o sample
#./sample
mpirun -np 4 -hostfile hosts sample
