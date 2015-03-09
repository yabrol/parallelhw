mpicc -c blah.c -o blah.o
mpicc -c mw_api.c -o mw_api.o
mpicc -g blah.o mw_api.o -o blah -lgmp
#./sample
for i in `seq 2 7`;
do
	mpirun -np $i -hostfile hosts blah 4000000000
done
