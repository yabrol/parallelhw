gcc -c sample.c -o sample.o
gcc -c mw_api.c -o mw_api.o
gcc -g sample.o mw_api.o -o sample
./sample
#mpirun -np 4 -hostfile hosts sample
