#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N_TESTS 1000000

int main(int argc, char** argv) {
	// Initialize the MPI environment
	int sz,myid;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
	MPI_Comm_size(MPI_COMM_WORLD, &sz);

	// Requires only 2 processes
	if (sz !=  2) {
		fprintf(stderr, "Size must be equal to 2 for this program\n");
		MPI_Abort(MPI_COMM_WORLD, 1); 
	}
  
	MPI_Status status;
	int i = N_TESTS;
	int sender_id = 0;
	int receiver_id =1;
	double total_time,average_time, t_send;
	char msg='x';
	while(i>0){
			if (myid == sender_id) {
				double t_received, delta;	
				// time of sending the message
				t_send = MPI_Wtime();
				// send the message with a char
				MPI_Send(&msg, 1 , MPI_CHAR, receiver_id, 0, MPI_COMM_WORLD);
				// receive the acknowledgment containing the received time of receiver
				MPI_Recv(&t_received,1, MPI_DOUBLE, receiver_id, 0, MPI_COMM_WORLD,&status);
				//calculate the time difference
				delta = t_received-t_send;
				total_time += delta;
			} 
			else if (myid == receiver_id) {
				double t_received;
				// Receive the message from sender
				MPI_Recv(&msg, 1 , MPI_CHAR, sender_id, 0, MPI_COMM_WORLD,&status);
				// note the received time
				t_received = MPI_Wtime();
				// send the time back to sender
				MPI_Send(&t_received,1, MPI_DOUBLE, sender_id, 0, MPI_COMM_WORLD);
			}
			i--;
	}
	if(myid == 0){
		average_time = total_time/N_TESTS;
		printf("Average Latency: %.12f\n",average_time);
	}
  MPI_Finalize();
}
