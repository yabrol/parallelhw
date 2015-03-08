mpicc -c blah.c -o blah.o
mpicc -c mw_api.c -o mw_api.o
mpicc -g blah.o mw_api.o -o blah -lgmp
#./sample
mpirun -np 5 -hostfile hosts blah 6400
