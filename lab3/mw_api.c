#include "mw_api.h"
#include "queue.h"
#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TRUE 1
#define FALSE 0
#define MASTER_ID 0
#define TAG_WORK 0
#define TAG_RESULT 1
#define TAG_TERMINATE 99
#define TAG_HEARTBEAT 2

int random_fail();
int F_Send(void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, int myid);

void send_heartbeat ()
{
	printf("Send heartbeat\n");
	char beat = 'a';
	MPI_Send(&beat, 1, MPI_BYTE, MASTER_ID, TAG_HEARTBEAT, MPI_COMM_WORLD);
}

void send_work(int wid,work_queue wq,struct mw_api_spec *f){
	int size;
	work_unit *chunk = (work_unit *)malloc(f->work_sz);
	chunk = wq->front->work;
	unsigned char *serialized_chunk = f->serialize(chunk,&size);
	//printf("Serializing done %lu\n",(int)(*serialized_chunk));
	MPI_Send(serialized_chunk, size, MPI_CHAR, wid, TAG_WORK, MPI_COMM_WORLD );
	//printf("Process %d out of %d\n", wid, sz);
	free(serialized_chunk);
	free(chunk);
	dequeue(wq);
}

void MW_Run (int argc, char **argv, struct mw_api_spec *f){
	int sz, myid;
	MPI_Comm_size(MPI_COMM_WORLD, &sz);
  	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
	MPI_Status status;

	if(myid == MASTER_ID){
		// Get pool of work
		work_unit **work;
		work = f->create(argc,argv);
		work_queue wq;
		wq = queue_create();
		// put work into the queue
		int i=0;
		while(work[i]!=NULL){
			enqueue(wq,work[i]);
			i++;
		}
		int wid=1;
		int n_chunks;
		int size;
		double start_time, end_time, delta;
		// Have queues for work units to be done
		// Send chunks of work to all the workers unless you encounter null
		start_time = MPI_Wtime();
		for(wid=1;wid<sz;wid++){
			send_work(wid,wq,f);
		}
		n_chunks = i;
		// Wait for the results
		result_unit **results = (result_unit **)malloc((n_chunks)*sizeof(result_unit *));
		i=0;
		int new_results_to_fetch = sz-1;
		printf("sz %d\n",new_results_to_fetch);
		
		while(new_results_to_fetch){
			int result_size;
			MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			MPI_Get_count(&status, MPI_BYTE, &result_size);
			wid = status.MPI_SOURCE;
			if(status.MPI_TAG == TAG_RESULT){
				unsigned char *serialized_result = (unsigned char *)malloc(result_size);
				MPI_Recv(serialized_result, result_size, MPI_BYTE, wid, TAG_RESULT, MPI_COMM_WORLD, &status);
				new_results_to_fetch--;
				result_unit *r = (result_unit *)malloc(f->res_sz);
				
				// deserialize result
			  	r = f->deserialize_result(serialized_result,result_size);	
				printf("done %d\n",wid);
				results[i]=r;
				i++;

				if(queue_empty(wq) == FALSE){
			  		send_work(wid,wq,f);
			  		new_results_to_fetch++;
			  	}
		  	}
		  	else if(status.MPI_TAG == TAG_HEARTBEAT){
		  		char beat;
		  		MPI_Recv(&beat, result_size, MPI_BYTE, wid, TAG_HEARTBEAT, MPI_COMM_WORLD, &status);
		  		printf("processor %d alive and sent %c\n",wid,beat);
		  	}
		}

		// terminate all workers
		for(i=1;i<sz;i++){
			work_unit *chunk = (work_unit *)malloc(f->work_sz);
			MPI_Send(chunk, f->work_sz, MPI_BYTE, i, TAG_TERMINATE, MPI_COMM_WORLD );
			free(chunk);
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
		result_unit *w_r,*temp_result;
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
				w_r = f->get_result_object();
				temp_result = f->get_result_object();
				//printf("start deserializing\n");
				w_work = f->deserialize(serialized_work,size);
				/*
				get back a result object which has the information whether it's a partial or complete result
				if it's a partial result send out heartbeat and resume work
				*/
				unsigned char *serialized_result;
				int len;

				while(TRUE){
					temp_result = f->compute(w_work);
					// combine results
					w_r = f->combine_partial_results(temp_result,w_r);
					if(f->get_result_state(temp_result) == 1){
						printf("completed work and first is %lu for processor %d\n",f->work_first(w_work),myid);
						break;
					}	
					else{
						// send heartbeat
						send_heartbeat();
						printf("partially complete and work first after partial result %lu for processor %d\n",f->work_first(w_work),myid);
					}
				}
				// Send it back to the master
				// serialize result
				serialized_result = f->serialize_result(w_r,&len);

				//printf("serialized length %d\n",(int)(*serialized_result));
				//MPI_Send(serialized_result, len, MPI_BYTE, MASTER_ID, TAG_RESULT, MPI_COMM_WORLD);
				//FAIL THE WORKER
				F_Send(serialized_result, len, MPI_BYTE, MASTER_ID, TAG_RESULT, MPI_COMM_WORLD, myid);
				free(w_r);
				free(temp_result);
				free(w_work);
			}
			// if termination tag received cleanup
			if(status_w.MPI_TAG == TAG_TERMINATE){
				free(w_work);
				break;
			}
		}
		printf("terminate %d\n",myid);
	}
}

int F_Send(void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, int myid)
{
	if (random_fail(myid)) {
		printf("%d going down\n", myid);      
		MPI_Finalize();
		exit (0);
		return 0;
   } else {
      return MPI_Send (buf, count, datatype, dest, tag, comm);
   }
}

//can be implemented using the built-in random number generator and comparing the value returned to the threshold p
int random_fail(int myid){
	// get random number seeded by system time and process rank
	// source: http://scicomp.stackexchange.com/questions/1274/how-can-i-seed-a-parallel-linear-congruential-pseudo-random-number-generator-for
	srand(abs(((time(NULL)*181)*((myid-83)*359))%104729));
	int randomNum = rand() % 101;

	int p = 80;//change as needed

	if(randomNum > p)
	{
		return TRUE;
	} else{
		return FALSE;
	}
}
