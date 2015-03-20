#include "mw_api.h"
#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <time.h>

#define TRUE 1
#define MASTER_ID 0
#define TAG_WORK 0
#define TAG_RESULT 1
#define TAG_TERMINATE 99

void MW_Run (int argc, char **argv, struct mw_api_spec *f){
	int sz, myid;
	MPI_Comm_size(MPI_COMM_WORLD, &sz);
  	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
	MPI_Status status;
	if(myid == MASTER_ID){
		// Get pool of work
		work_unit **work;// = (work_unit *)malloc(f->work_sz);
		work = f->create(argc,argv);
		int wid=1;
		int n_chunks;
		int size;
		double start_time, end_time, delta;
		// Send chunks of work to all the workers unless you encounter null
		int i=0;
		start_time = MPI_Wtime();
		while(work[i]!=NULL){
			work_unit *chunk = (work_unit *)malloc(f->work_sz);
			chunk = work[i];
			unsigned char *serialized_chunk = f->serialize(work[i],&size);
			//printf("Serializing done %lu\n",(int)(*serialized_chunk));
			MPI_Send(serialized_chunk, size, MPI_CHAR, wid, TAG_WORK, MPI_COMM_WORLD );
			//printf("Process %d out of %d\n", wid, sz);
			wid = 1 + (wid)%(sz-1);
			i++;
			free(serialized_chunk);
			free(chunk);
		}
		n_chunks = i;
		//printf("total_workers %d\n",sz-1);
		// Wait for the results
		result_unit **results = (result_unit **)malloc((n_chunks)*sizeof(result_unit *));
		wid=0;
		for(i=0;i<n_chunks;i++){
			
			result_unit *r = (result_unit *)malloc(f->res_sz);
			wid = 1 + (wid)%(sz-1);
			//printf("wait %d\n",wid);
			int result_size;
			MPI_Probe(wid, TAG_RESULT, MPI_COMM_WORLD, &status);
			MPI_Get_count(&status, MPI_BYTE, &result_size);

			unsigned char *serialized_result = (unsigned char *)malloc(result_size);
			MPI_Recv(serialized_result, result_size, MPI_BYTE, wid, TAG_RESULT, MPI_COMM_WORLD, &status);
			// deserialize result
		  	r = f->deserialize_result(serialized_result,result_size);	
			//printf("done %d\n",wid);
			results[i]=r;
		
			//free(r);
		}
		// terminate all workers
		for(i=1;i<sz;i++){
			work_unit *chunk;
			MPI_Send(chunk, f->work_sz, MPI_BYTE, i, TAG_TERMINATE, MPI_COMM_WORLD );
		}
		// compile the results together
		int compilation_status=0;
		compilation_status = f->compile(n_chunks,results);
		//printf("compilation %d\n",compilation_status);
		free(results);
		end_time = MPI_Wtime();
		delta = end_time - start_time;
		printf("%.11f\n",delta);
	}
	else{
		MPI_Status status_w,status_size;
		work_unit *w_work;
		result_unit *w_r;
		int size;
		while(TRUE){
			// http://mpitutorial.com/tutorials/dynamic-receiving-with-mpi-probe-and-mpi-status/
			MPI_Probe(MASTER_ID, MPI_ANY_TAG, MPI_COMM_WORLD, &status_size);
			MPI_Get_count(&status_size, MPI_BYTE, &size);
			w_work = (work_unit *)malloc(f->work_sz);
			//printf("size message to receive%d\n",size);
			unsigned char *serialized_work = (unsigned char *)malloc(sizeof(unsigned char)*size);
			// Receive chunks of work
			MPI_Recv(serialized_work,size, MPI_BYTE, MASTER_ID, MPI_ANY_TAG, MPI_COMM_WORLD, &status_w);
			// if work tag received
			// Compute the results 
			if(status_w.MPI_TAG == TAG_WORK){
				w_r = (result_unit *)malloc(f->res_sz);
				//printf("start deserializing\n");
				w_work = f->deserialize(serialized_work,size);
				w_r = f->compute(w_work);
				
				int len;
				// Send it back to the master
				// serialize result
				unsigned char *serialized_result  = f->serialize_result(w_r,&len);
				//printf("serialized length %d\n",(int)(*serialized_result));
				//temp remove this next line
				MPI_Send(serialized_result, len, MPI_BYTE, MASTER_ID, TAG_RESULT, MPI_COMM_WORLD);
				//FAIL THE WORKER
				//F_Send(serialized_result, len, MPI_BYTE, MASTER_ID, TAG_RESULT, MPI_COMM_WORLD);
				free(w_r);
				free(w_work);
			}
			// if termination tag received cleanup
			if(status_w.MPI_TAG == TAG_TERMINATE){
				//printf("terminate %d\n",myid);
				free(w_work);
				break;
			}
		}
	}
}

int F_Send(void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
{
	if (random_fail()) {      
		MPI_Finalize();
		exit (0);
		return 0;
   } else {
      returm MPI_Send (buf, count, datatype, dest, tag, comm);
   }
}

//can be implemented using the built-in random number generator and comparing the value returned to the threshold p
bool random_fail(){
	//get random number seeded by system time
	srand(time(NULL));//if things break, we can remove this line so seed will always be 1, but each time same numbers will be generated
	int randomNum = rand() % 101;

	int p = 80;//change as needed

	if(randomNum > p)
	{
		return TRUE;
	} else{
		return FALSE;
	}
}

