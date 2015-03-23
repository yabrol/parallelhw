#include "mw_api.h"
#include "queue.h"
#include "resultQueue.h"
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
#define BUSY 1
#define IDLE 2
#define DEAD 0

int random_fail();
typedef struct processor_t{
	int pid;
	int status;
	int work_id;
	int is_master;
	double last_seen;
	int missed_count;
} processor;

processor init_processor(int pid,int status,int work_id,int is_master){
	processor p;
	p.pid = pid;
	p.status = status;
	p.work_id = work_id;
	p.is_master = is_master;
	p.last_seen = MPI_Wtime();
	p.missed_count = 0;
	return p;
}

int is_any_processor_busy(processor processors[]){
	int i=0;
	double timeout = .00050;
	while(processors[i].pid!=-1){
		processor p = processors[i];
		if((p.is_master == FALSE) && (p.status!= IDLE) && (p.status!= DEAD)){
			return TRUE;
		}
		i++;
	}
	return FALSE;
}

void get_lost_work(processor processors[],work_queue pwq, work_queue wq,double current_time){
	int i=0;
	double timeout = .00050;
	while(processors[i].pid!=-1){
		processor p = processors[i];
		
		if((p.is_master == FALSE) && (p.status!= DEAD)){
			printf("checking for %d %.5f %.5f\n",p.pid,MPI_Wtime(),p.last_seen);
		
			if((current_time - p.last_seen) > timeout){
					processors[i].missed_count += 1;
				if(processors[i].missed_count >1){
					processors[i].status = DEAD;

					printf("Processor %d failed and moving work %d\n", p.pid,p.work_id);
					work_node *lost_work = dequeue_by_id(pwq,p.work_id);
					enqueue(wq, lost_work);
				}
				
				
			}
			else{
				printf("I'm alive!!!!!!!!!!!!!!!!!!\n");
			}
		}
		i++;
	}
}

int F_Send(void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, int myid);

void send_heartbeat (int myid)
{
	printf("Send heartbeat %d\n",myid);
	char beat = 'a';
	F_Send(&beat, 1, MPI_BYTE, MASTER_ID, TAG_HEARTBEAT, MPI_COMM_WORLD, myid);
}



void send_work(int wid,work_queue wq,struct mw_api_spec *f, processor processors[], work_queue pwq){
	int size;
	work_unit *chunk = (work_unit *)malloc(f->work_sz);
	chunk = wq->front->work;
	unsigned char *serialized_chunk = f->serialize(chunk,&size);

	//printf("Serializing done %lu\n",(int)(*serialized_chunk));
	MPI_Send(serialized_chunk, size, MPI_CHAR, wid, TAG_WORK, MPI_COMM_WORLD );
	//printf("Process %d out of %d\n", wid, sz);
	free(serialized_chunk);
	free(chunk);
	processors[wid] =  init_processor(wid,BUSY,wq->front->id,FALSE);
	work_node *temp = dequeue(wq);
	printf("temp->id %d\n",temp->id);
	enqueue(pwq,temp);
}

void MW_Run (int argc, char **argv, struct mw_api_spec *f){
	int sz, myid;
	MPI_Comm_size(MPI_COMM_WORLD, &sz);
  	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
	MPI_Status status;

	if(myid == MASTER_ID){
		// Get pool of work
		processor processors[sz+1];
		processor master = init_processor(MASTER_ID,BUSY,-1,TRUE);
		processors[MASTER_ID] = master;
		processors[sz] = init_processor(-1,BUSY,-1,TRUE);
		work_unit **work;
		work = f->create(argc,argv);
		work_queue wq,pwq;
		wq = queue_create();
		pwq = queue_create();
		// put work into the work queue
		int i=0;
		while(work[i]!=NULL){
			work_node *temp_work_node = get_work_node(work[i],i);
			enqueue(wq,temp_work_node);
			i++;
		}
		int wid=1;
		int n_chunks;
		int size;
		double start_time, end_time, delta;
		// Have queues for work units to be done
		// Send chunks of work to all the workers unless you encounter null
		for(wid=1;wid<sz;wid++){
			send_work(wid,wq,f,processors,pwq);
		}
		n_chunks = i;
		// Wait for the results=
		result_unit **results = (result_unit **)malloc((n_chunks)*sizeof(result_unit *));
		result_queue rq;
		rq = rQueue_create();
		i=0;
		int new_results_to_fetch = sz-1;
		double current_time;
		printf("sz %d\n",new_results_to_fetch);
		start_time = MPI_Wtime();
		double timeout = .0010;
		
		while(new_results_to_fetch){

			int result_size;
			int flag=0;

			while(!flag){
				current_time = MPI_Wtime();
				if( (current_time - start_time) > timeout ){
					// check for dead workers and change their status
					//work_lost = 
					get_lost_work(processors,pwq,wq,current_time);
					start_time = current_time;
					// get the work pice from processed queue and put it back on to work queue
				}
				if(is_any_processor_busy(processors) == FALSE){
					break;
				}
				MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
			}
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
				processors[wid].status = IDLE;
				if(queue_empty(wq) == FALSE){
			  		send_work(wid,wq,f,processors,pwq);
			  		new_results_to_fetch++;
			  	}
		  	}
		  	else if(status.MPI_TAG == TAG_HEARTBEAT){
		  		char beat;
		  		MPI_Recv(&beat, result_size, MPI_BYTE, wid, TAG_HEARTBEAT, MPI_COMM_WORLD, &status);
		  			printf("recv heartbeat %d\n",wid);
		  		// update the worker array
		  		processors[wid].last_seen = MPI_Wtime(); 
		  		printf("processor %d %.11f alive and sent %c\n",processors[wid].pid,processors[wid].last_seen,beat);
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
						// printf("completed work and first is %lu for processor %d\n",f->work_first(w_work),myid);
						break;
					}	
					else{
						// send heartbeat
						send_heartbeat(myid);
						// printf("partially complete and work first after partial result %lu for processor %d\n",f->work_first(w_work),myid);
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

	int p = 70;//change as needed

	if(randomNum > p)
	{
		return TRUE;
	} else{
		return FALSE;
	}
}
