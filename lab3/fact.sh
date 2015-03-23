mpicc -c sample_q2.c -o sample_q2.o
mpicc -c queue.c -o queue.o
mpicc -c mw_api.c -o mw_api.o
mpicc -g sample_q2.o mw_api.o queue.o -o sample_q2 -lgmp
#./sample
for i in `seq 5 5`;
do
	mpirun -np $i sample_q2 40000000000
done
