#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int N_TESTS = 2;

int main(int argc, char** argv) {
  // Initialize the MPI environment
  MPI_Init(NULL, NULL);
  // Find out rank, size
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  // We are assuming at least 2 processes for this task
  if (world_size < 2) {
    fprintf(stderr, "World size must be greater than 1 for %s\n", argv[0]);
    MPI_Abort(MPI_COMM_WORLD, 1); 
  }
  
	MPI_Status status;
	int i = N_TESTS;
	double total_time;	
	double average_time;
	int rc2;
	double t_send,t_received;
	int rc;
	double delta;
	char msg='x';
	while(i>0){
			if (world_rank == 0) {
				// If we are rank 0, set the number to -1 and send it to process 1
				t_send = MPI_Wtime();
				rc2 = MPI_Send(&msg, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
				if (rc2 != MPI_SUCCESS) {
						 printf("Send error in task 0!\n");
						 MPI_Abort(MPI_COMM_WORLD, rc2);
						 exit(1);
				}
				else if(rc2 == MPI_SUCCESS){
						//printf("Sent succesfully\n");
						MPI_Recv(&t_received,8, MPI_INT, 1, 0, MPI_COMM_WORLD,&status);
						delta = t_received-t_send;
						//printf("Time Difference %f\n",delta);
						total_time += delta;
				}
			} 
			else if (world_rank == 1) {
				rc = MPI_Recv(&msg, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,&status);
				if(rc ==MPI_SUCCESS){
						t_received = MPI_Wtime();
						MPI_Send(&t_received,8, MPI_INT, 0, 0, MPI_COMM_WORLD);
				}
				else if (rc != MPI_SUCCESS) {
						 printf("Receive error in task 1!\n");
						 MPI_Abort(MPI_COMM_WORLD, rc);
						 exit(1);
				}
			}
			i--;
	}
	average_time = total_time/N_TESTS;
	printf("Average Latency: %f\n",average_time);
  MPI_Finalize();
}
