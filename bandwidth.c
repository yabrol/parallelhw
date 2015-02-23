#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>

int main (int argc, char **argv)
{
	int sz, myid;
	//initialize MPI
	MPI_Init (&argc, &argv);
	//get total number of processes
	MPI_Comm_size (MPI_COMM_WORLD, &sz);
	//get id of each process
	MPI_Comm_rank (MPI_COMM_WORLD, &myid);
	
	int tests = 100;
	int max_messages = 1000000;
	int sender_id = 0;
	int receiver_id = 1;
	int n_messages=10000,step=100000;
	
	while(n_messages<max_messages){
		double total_bandwidth=0;

		if(myid == sender_id){
			int ack,test = tests;
			int tag =0;
			MPI_Status status;
			// run tests for the given number of messages
			// construct message as double to be able to store time
			double *msg = (double *)malloc(n_messages * sizeof(double));
			int i =0;
			for(i=0;i<n_messages;i++){
				msg[i] = i*1.0;
			}
			while(test>0){
				// insert the current sending time
				msg[0] = MPI_Wtime();
				// send packet to receiver
				MPI_Send(msg ,n_messages ,MPI_DOUBLE ,receiver_id , tag, MPI_COMM_WORLD);
				// wait for acknowledgment from receiver
				MPI_Recv(&ack, 1, MPI_INT, receiver_id, tag, MPI_COMM_WORLD, &status);

				test--;
			}
			free(msg);
		}
		else if(myid == receiver_id){
			int test = tests;
			int ack,tag =0;
			double time_received,delta;
			double bandwidth=0;
			double *msg = (double *)malloc(n_messages * sizeof(double));
			MPI_Status status;
			while(test >0){
				// receive packets from sender
				MPI_Recv(msg, n_messages, MPI_DOUBLE, sender_id, tag, MPI_COMM_WORLD, &status);
				// get current time
				time_received = MPI_Wtime();
				// calculate the bandwidth using time difference and total size of the message
				delta = time_received - msg[0];
				bandwidth = (sizeof(double)* n_messages)/delta;
				// add it to the total of bandwidths
				total_bandwidth += bandwidth;
				// send acknowledgment to sender
				MPI_Send(&ack, 1, MPI_INT, sender_id, tag, MPI_COMM_WORLD);
				test--;
			}
			printf("Average bandwidth: %f for %d messages and %d tests\n",(total_bandwidth/(tests*1024*1024)),n_messages,tests);
			free(msg);
		}

		n_messages += step;
	}

	MPI_Finalize ();
	exit (0);
}
