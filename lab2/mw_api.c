#include "mw_api.h"
#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>

void testing(){
	printf("testing\n");
}
#define TRUE 1
#define MASTER 0
#define TAG_WORK 0
#define TAG_RESULT 1

void MW_Run (int argc, char **argv, struct mw_api_spec *f){
	int sz, myid;
	MPI_Comm_size(MPI_COMM_WORLD, &sz);
  	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
	MPI_Status status,status1;
	if(myid == MASTER)
	{
		// Get pool of work
		work_unit **work;// = (work_unit *)malloc(f->work_sz);
		work = f->create(argc,argv);
		int wid=1;
		int n_chunks,total_workers=0;
		// Send chunks of work to all the workers unless you encounter null
		int i=0;
		while(work[i]!=NULL){
			work_unit *chunk = (work_unit *)malloc(f->work_sz);
			chunk = work[i];
			MPI_Send(chunk, f->work_sz, MPI_BYTE, wid, TAG_WORK, MPI_COMM_WORLD );
			printf("Process %d out of %d\n", wid, sz);
			wid = 1 + (wid)%(sz-1);
			i++;
			free(chunk);
		}
		n_chunks = i;
		total_workers= sz-1;
		// Terminate workers

		printf("total_workers %d\n",total_workers);
		// Wait for the results
		result_unit **results = (result_unit **)malloc((n_chunks)*sizeof(result_unit *));
		wid=0;
		for(i=0;i<n_chunks;i++){
			
			result_unit *r = (result_unit *)malloc(f->res_sz);
			wid = 1 + (wid)%(sz-1);
			printf("wait %d\n",wid);
			MPI_Recv(r, f->res_sz, MPI_BYTE, wid, TAG_RESULT, MPI_COMM_WORLD, &status);
			printf("done %d\n",wid);
			results[i]=r;
		
			//free(r);
		}
		// compile the results together
		int compilation_status=0;
		compilation_status = f->compile(n_chunks,results);
		printf("compilation %d\n",compilation_status);
		free(results);
	}
	else{
		
		work_unit *w_work;
		result_unit *w_r;
		while(TRUE)
		{
			w_work = (work_unit *)malloc(f->work_sz);
			// Receive chunks of work
			MPI_Recv(w_work, f->work_sz, MPI_BYTE, MASTER, TAG_WORK, MPI_COMM_WORLD, &status1);
			// Compute the results
			w_r = (result_unit *)malloc(f->res_sz);
			w_r = f->compute(w_work);
			// Send it back to the master
			MPI_Send(w_r, f->res_sz, MPI_BYTE, MASTER, TAG_RESULT, MPI_COMM_WORLD);
			free(w_work);
			free(w_r);
		}
	}
}
