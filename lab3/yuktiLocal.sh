mpicc -c sample_q2.c -o sample_q2.o
mpicc -c mw_api.c -o mw_api.o
mpicc -c queue.c -o queue.o
mpicc -c resultQueue.c -o resultQueue.o
mpicc -c fileio.c -o fileio.o
mpicc -g sample_q2.o mw_api.o queue.o resultQueue.o fileio.o -o sample_q2 -lgmp

mpirun -np 4 sample_q2 900000000000
#./sample
# for i in `seq 5 5`;
# do
# 	mpirun -np $i -hostfile hosts sample_q2 900000000000
# done
