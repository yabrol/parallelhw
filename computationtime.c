#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

  int length = 20;
  double arrayA[length], arrayB[length], arrayC[2];
  double result;
  int numFloat = length*5;
  int rc2, max;
  double runningSum = 0;

  //initialize the arrays
  for( int i=0; i < length; i++){
  	arrayA[i] = i;
  	arrayB[i] = i + 1;
  	printf("%f\n", arrayA[i]);
  }

  if (world_rank == 0) {
  	//is master
  	max = length%world_size;
  	for(int i = 0; i < max; i++){

  	}
  	rc2 = MPI_Send(&msg, 1 , MPI_INT, 1, 0, MPI_COMM_WORLD);

  	//recieve
  	runningSum += MPI_Recv()
  }
  else{
  	//is slave
  }

  result = dotProduct(&arrayA, &arrayB, length);

  printf("num floating point %d\n", numFloat);
  printf("num computations %f\n", result/numFloat);

  MPI_Finalize();
}

/* Assume that a and b point to array with at least length elements */
/* Assume that none of the intermediate values overflows a double. */
 
 double dotProduct(double *a, double *b, int length) {
    
    for (int index = 0; index < length; index++)
        runningSum += a[index] * b[index];
    return runningSum;
 }

