mpicc -c mw_api.c -o mw_api.o
mpicc -c sample.c -o sample.o
mpicc mw_api.o sample.o -o sample
#./sample
mpirun -np 4 -hostfile hosts sample
