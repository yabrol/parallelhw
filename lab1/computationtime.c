#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_LENGTH 100000
#define TAG_A 0
#define TAG_B 1
#define TAG_O 2

// Assume that a and b point to array with at least length elements. 
// Assume that none of the intermediate values overflows a double. 
double dotProduct(double *a, double *b, int length) {
	int index = 0;
	double runningSum=0.0;
	for (index = 0; index < length; index++)
	runningSum += a[index] * b[index];
	return runningSum;
}


int main(int argc, char** argv) {

	int sz, myid;
  	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD, &sz);
  	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
	int master_id = 0;
	double runningSum = 0;
	MPI_Status status,status1,status2;
	
	if(sz >= 2){
		if (myid == master_id) {
			//is master
			int n_chunks = (sz-1);	
			double flops, output;
			double *arrayA = (double *)malloc((sizeof(double) * ARRAY_LENGTH));
			double *arrayB = (double *)malloc((sizeof(double) * ARRAY_LENGTH));

			int i,wid;
			int chunk_size = (int)((ARRAY_LENGTH)/(sz-1));
			double start_time, end_time, delta;
			// initialize arrays
			for(i = 0; i < ARRAY_LENGTH; i++){
				arrayA[i] = (double)1.0;
				arrayB[i] = (double)1.0;
			}
			// Computation time started
			start_time = MPI_Wtime();
			// break into chunks and send it workers
			for(wid=1;wid<sz;wid++){
				int x = chunk_size * (wid-1);
				// Handling the case where the array length cannot be equally divided into chunks
				if(wid == (sz -1)){
					chunk_size = chunk_size + ((ARRAY_LENGTH)%(sz-1));
				}
				MPI_Send(arrayA+x, chunk_size, MPI_DOUBLE, wid, TAG_A, MPI_COMM_WORLD );
				MPI_Send(arrayB+x, chunk_size, MPI_DOUBLE, wid, TAG_B, MPI_COMM_WORLD );
			}
			
			// receive output from workers
			for(wid=1;wid<sz;wid++){
				MPI_Recv(&output, 1, MPI_DOUBLE, wid, TAG_O, MPI_COMM_WORLD, &status);
				runningSum+=output;
			}
			// computation ended
			end_time = MPI_Wtime();

			delta = end_time - start_time;
			flops = (double)(2*ARRAY_LENGTH+sz)/delta;
			printf("%f,%f\n",flops,runningSum);
			free(arrayA);
			free(arrayB);
		}
		else{
		//is slave
			double result;
			int chunk_size = (int)((ARRAY_LENGTH)/(sz-1));
			// Handling the case where the array length cannot be equally divided into chunks
			if(myid == (sz -1)){
				chunk_size = chunk_size + ((ARRAY_LENGTH)%(sz-1));
			}
			double *a = (double *)malloc((sizeof(double) * chunk_size));
			double *b = (double *)malloc((sizeof(double) * chunk_size));
			// receive array form master
			MPI_Recv(a, chunk_size, MPI_DOUBLE, master_id, TAG_A, MPI_COMM_WORLD, &status1);
			MPI_Recv(b, chunk_size, MPI_DOUBLE, master_id, TAG_B, MPI_COMM_WORLD, &status2);
			// calculate dot product
			result = dotProduct(a, b, chunk_size);
			// send to master
			MPI_Send(&result, 1, MPI_DOUBLE, master_id, TAG_O, MPI_COMM_WORLD);
			free(a);
			free(b);
		}
	}
	// if size is 1, the node does all the work
	else if(sz == 1){
		double *arrayA = (double *)malloc((sizeof(double) * ARRAY_LENGTH));
		double *arrayB = (double *)malloc((sizeof(double) * ARRAY_LENGTH));
		int i;
		for(i = 0; i < ARRAY_LENGTH; i++){
				arrayA[i] = (double)1.0;
				arrayB[i] = (double)1.0;
		}
		double start_time = MPI_Wtime();
		double product = dotProduct(arrayA,arrayB,ARRAY_LENGTH);
		double end_time = MPI_Wtime();
		double flops = (double)(2*ARRAY_LENGTH)/(end_time-start_time);
		printf("%f, %f\n",flops,product);
		free(arrayA);
		free(arrayB);
	}
	else{
		printf("Total nodes must be greater than 1\n");
    		MPI_Abort(MPI_COMM_WORLD, 1); 
	}
	MPI_Finalize();
}
