mpicc -c sample.c -o sample.o
mpicc -c mw_api.c -o mw_api.o
mpicc -g sample.o mw_api.o -o sample
#./sample
mpirun -np 5 -hostfile hosts sample
