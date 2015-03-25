#include "mw_api.h"
#include "queue.h"
#include "fileio.h"
#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TRUE 1
#define FALSE 0
#define TAG_WORK 0
#define TAG_RESULT 1
#define TAG_TERMINATE 99
#define TAG_HEARTBEAT 2
#define TAG_MASTER_DIED 404
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

int get_idle_processor(processor processors[]){
	int i=0;
	while(processors[i].pid!=-1){
		processor p = processors[i];
		if((p.is_master == FALSE) && (p.status == IDLE)){
			return p.pid;
		}
		i++;
	}
	return -1;
}

int is_any_processor_busy(processor processors[]){
	int i=0;
	while(processors[i].pid!=-1){
		processor p = processors[i];
		if((p.is_master == FALSE) && (p.status!= IDLE) && (p.status!= DEAD)){
			return TRUE;
		}
		i++;
	}
	return FALSE;
}

int is_any_processor_alive(processor processors[]){
	int i=0;
	while(processors[i].pid!=-1){
		processor p = processors[i];
		if((p.is_master == FALSE) && (p.status!= DEAD)){
			return TRUE;
		}
		i++;
	}
	return FALSE;
}


void terminate_workers(processor processors[], int work_sz){
	int i=0;
	while(processors[i].pid!=-1){
		processor p = processors[i];
		if((p.is_master == FALSE) && (p.status!= DEAD)){
			work_unit *chunk = (work_unit *)malloc(work_sz);
			MPI_Send(chunk, work_sz, MPI_BYTE, p.pid, TAG_TERMINATE, MPI_COMM_WORLD );
			processors[i].status = DEAD;
			free(chunk);	
		}
		i++;
	}
}


void get_lost_work(processor processors[],work_queue pwq, work_queue wq,double current_time, int *n_results, work_unit **work,struct mw_api_spec *f){
	int i=0;
	double timeout = .00100;
	while(processors[i].pid!=-1){
		processor p = processors[i];
		
		if((p.is_master == FALSE) && (p.status!= DEAD) && (p.status!= IDLE)){
			printf("checking for %d %.5f %.5f\n",p.pid,MPI_Wtime(),p.last_seen);
		
			if((current_time - p.last_seen) > timeout){
					processors[i].missed_count += 1;
				if(processors[i].missed_count >1){
					processors[i].status = DEAD;
		  			printf("terminate cause assumed dead\n");
					work_unit *chunk = (work_unit *)malloc(f->work_sz);
					MPI_Send(chunk, f->work_sz, MPI_BYTE, p.pid, TAG_TERMINATE, MPI_COMM_WORLD );
					
					free(chunk);
					(*n_results)--;
					printf("Processor %d failed and moving work %d\n", p.pid,p.work_id);
					//print_queue(pwq);
					//work_node *temp = dequeue_by_id(pwq,p.work_id);
					//print_queue(wq);
					f->reinit(work[p.work_id]);
					work_node *temp = get_work_node(work[p.work_id],p.work_id);
					printf("get work first at transfer %lu %lu\n", f->work_first(temp->work), f->work_first(work[p.work_id]));
					enqueue(wq, temp);
					print_queue(wq);

				}	
			}
			else{
				printf("I'm alive!\n");
			}
			printf("new_results_to_fetch in check %d\n", *n_results);

		}
		i++;
	}
}

int F_Send(void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, int myid);

void send_heartbeat (int myid, int dest)
{
	printf("Send heartbeat %d\n",myid);
	char beat = 'a';
	F_Send(&beat, 1, MPI_BYTE, dest, TAG_HEARTBEAT, MPI_COMM_WORLD, myid);
}



void send_work(int wid,work_queue wq,struct mw_api_spec *f, processor processors[], work_queue pwq, int source){
	int size;
	work_unit *chunk = (work_unit *)malloc(f->work_sz);
	chunk = wq->front->work;
	//work_node *temp = dequeue(wq);

	printf("temp->id %d\n",wq->front->id);
	//chunk = temp->work;
	printf("dequeued work first %lu\n",f->work_first(wq->front->work));
	unsigned char *serialized_chunk = f->serialize(chunk,&size);

	printf("Serializing done %d\n",(int)(*serialized_chunk));
	//free(temp);
	printf("temp freed ok and size %d\n",size);
	F_Send(serialized_chunk, size, MPI_BYTE, wid, TAG_WORK, MPI_COMM_WORLD, source);
	//printf("Process %d out of %d\n", wid, sz);
	free(serialized_chunk);
	//free(chunk);
	//processors[wid] =  init_processor(wid,BUSY,wq->front->id,FALSE);
	processors[wid].status = BUSY;
	dequeue(wq);

	//enqueue(pwq,temp);
}

void write_work_to_file( work_unit **work, int count, struct mw_api_spec *f){
	write_workList_length(count);
	int i = 0;
	int size;
	while(work[i]){
		unsigned char *serialized_chunk = f->serialize(work[i],&size);
		write_workList(serialized_chunk,size);
		i++;
	}
}



void master(int sz,int argc, char **argv, struct mw_api_spec *f, int m_id){
		MPI_Status status;
		processor processors[sz+1];
		processor master = init_processor(m_id,BUSY,-1,TRUE);
		processors[m_id] = master;
		processors[sz] = init_processor(-1,BUSY,-1,TRUE);
		work_unit **work;
		work_unit **backup_work;
		work_unit **temp_work;

		if(m_id == 0){
			temp_work = f->create(argc,argv);
			// write work to file
			int n_work = 0;
			while(temp_work[n_work]!=NULL){
				n_work++;
			}
			n_work--;
			write_work_to_file(temp_work,n_work,f);
			work = read_work_list(f);
		}

		backup_work = f->create(argc,argv);
		work_queue wq,pwq;
		wq = queue_create();
		pwq = queue_create();
		int all_dead = FALSE;
		// put work into the work queue
		int i=0;
		while(work[i]!=NULL){
			work_node *temp_work_node = get_work_node(work[i],i);
			enqueue(wq,temp_work_node);
			printf(" %d has first %lu\n", i, f->work_first(wq->front->work));
			i++;
		}
		print_queue(wq);
		
		while(queue_empty(wq)==FALSE){

			enqueue(pwq,wq->front);
			dequeue(wq);
		}
		print_queue(pwq);
		while(queue_empty(pwq)==FALSE){

			enqueue(wq,pwq->front);
			
			printf(" has first %lu\n", f->work_first(wq->front->work));
			dequeue(pwq);
		}
		print_queue(wq);
		int r=0;
		while(work[r]!=NULL){
			printf("at timeout work %d has first %lu\n", r, f->work_first(work[r]));
			r++;
		}
		
		int wid=1;
		int n_chunks;
		int size;
		double start_time, end_time, delta;
		// Have queues for work units to be done
		// Send chunks of work to all the workers unless you encounter null
		for(wid=0;wid<sz;wid++){
			if(wid!=m_id){
				//init the state
				processors[wid] =  init_processor(wid,IDLE,wq->front->id,FALSE);
			}
		}

		for(wid=0;wid<sz;wid++){
			if(wid!=m_id){
			send_work(wid,wq,f,processors,pwq,m_id);
			printf("dine sending work to %d\n",wid );
			}
		}
		n_chunks = i;
		// Wait for the results
		result_unit **results = (result_unit **)malloc((n_chunks)*sizeof(result_unit *));
		i=0;
		int new_results_to_fetch = sz-1;
		double current_time;
		printf("sz %d\n",new_results_to_fetch);
		start_time = MPI_Wtime();
		double timeout = .0010;
		
		while( ((new_results_to_fetch) || (queue_empty(wq) == FALSE)) && (all_dead == FALSE) ){
			//printf("new_results_to_fetch in loop %d\n", new_results_to_fetch);

			int result_size;
			int flag=0;

			while(!flag){
				current_time = MPI_Wtime();
				if( (current_time - start_time) > (5*timeout) ){
					// send heartbeat to workers

				}
				if( (current_time - start_time) > timeout ){
					// check for dead workers and change their status
					//work_lost = 
					// print the entire work array first
					int j=0;
					while(work[j]!=NULL){
						printf("at timeout work %d has first %lu\n", j, f->work_first(work[j]));
						j++;
					}
					get_lost_work(processors,pwq,wq,current_time,&new_results_to_fetch,work,f);
					printf("new_results_to_fetch %d\n", new_results_to_fetch);
					start_time = current_time;
					// get the work pice from processed queue and put it back on to work queue
				}

				if(queue_empty(wq) == FALSE){
			  		// check for idle processors
			  		// if no idle processors for now then continue
			  		// else if you find idle worker assign work to it
			  		wid = get_idle_processor(processors);
			  			//printf("idle processor %d\n",wid );

			  		if(wid != -1)
			  		{
			  			printf("assigning to wid %d\n",wid );

			  			send_work(wid,wq,f,processors,pwq,m_id);
			  			printf("assigned to wid %d\n",wid );
			  			new_results_to_fetch++;
			  		}
			  		else if(is_any_processor_alive(processors) == FALSE)
			  		{
			  			all_dead = TRUE;
			  			printf("All workers died \n");
			  			break;
			  		}
			  	}
				if(is_any_processor_busy(processors) == FALSE){
					break;
				}
				MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
			}
			if(flag){
				MPI_Get_count(&status, MPI_BYTE, &result_size);
				wid = status.MPI_SOURCE;
				if(status.MPI_TAG == TAG_RESULT && processors[wid].status!=DEAD){
					unsigned char *serialized_result = (unsigned char *)malloc(result_size);
					MPI_Recv(serialized_result, result_size, MPI_BYTE, wid, TAG_RESULT, MPI_COMM_WORLD, &status);
					int l=0;
			  		while(work[l]!=NULL){
						printf("work %d has first %lu after resulr from %d\n", l, f->work_first(work[l]), wid);
						l++;
					}
					new_results_to_fetch--;
					result_unit *r = (result_unit *)malloc(f->res_sz);
					// write serialized result to file
					write_result_list(serialized_result,result_size);
					// deserialize result
				  	r = f->deserialize_result(serialized_result,result_size);	
					printf("done %d\n",wid);
					results[i]=r;
					i++;
					processors[wid].status = IDLE;
			  	}
			  	else if(status.MPI_TAG == TAG_HEARTBEAT){

			  		char beat;
			  		MPI_Recv(&beat, result_size, MPI_BYTE, wid, TAG_HEARTBEAT, MPI_COMM_WORLD, &status);
			  			printf("recv heartbeat %d\n",wid);
			  		// update the worker array
			  		processors[wid].last_seen = MPI_Wtime();
			  		int k=0;
			  		while(work[k]!=NULL){
						printf("work %d has first %lu\n", k, f->work_first(work[k]));
						k++;
					} 
			  		printf("processor %d %.11f alive and sent %c\n",processors[wid].pid,processors[wid].last_seen,beat);

			  	}
		  	}
		}

		// terminate all workers
		terminate_workers(processors,f->work_sz);

		if(!all_dead){
			// compile the results together
			int compilation_status=0;
			compilation_status = f->compile(n_chunks,results);
		}	
		//printf("compilation %d\n",compilation_status);
		free(results);
		queue_destroy(wq);
		queue_destroy(pwq);
		end_time = MPI_Wtime();
		delta = end_time - start_time;
		printf("%.11f\n",delta);	
}

void send_all_processes(void *buf, int tag, int total_processes,int myid){
	MPI_Request request;
    int i=0;
    for(i=0;i<total_processes;i++){
    	if(i!=myid)
    		MPI_Send (buf, sizeof(int), MPI_BYTE, i, tag, MPI_COMM_WORLD);
	}
}

void MW_Run (int argc, char **argv, struct mw_api_spec *f){
	int sz, myid;
	MPI_Comm_size(MPI_COMM_WORLD, &sz);
  	MPI_Comm_rank(MPI_COMM_WORLD, &myid);

	if(myid == 0){
		// Get pool of work
		master(sz,argc,argv,f,0);

	}
	else{
		MPI_Status status_w,status_size;
		work_unit *w_work;
		result_unit *w_r,*temp_result;
		int size,flag=0;
		double start_time, current_time;
		double master_timeout = 1;
		int MASTER_ID = 0;
		int INFORMED_OTHERS = FALSE;
		int new_master_id = myid;
		int master_alive = TRUE;
		double timeout = master_timeout;

		while(TRUE){
			start_time = MPI_Wtime();
			master_alive = TRUE;
			flag = 0;
			// http://mpitutorial.com/tutorials/dynamic-receiving-with-mpi-probe-and-mpi-status/
			while(!flag){
				current_time = MPI_Wtime();
				//printf("processor %d wait\n", myid);
				if( (current_time - start_time) > timeout )
				{
					// master dead
					//if(myid == 1){
					printf("processor %d wait exceed %f\n", myid,timeout);
					master_alive = FALSE;

					//}
					
					break;
				}
				MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD ,&flag, &status_size);
			}
			if(master_alive == FALSE){
					if(INFORMED_OTHERS == TRUE){
						if(myid == MASTER_ID){
							printf("\n\n================master dead 1====================\n");
							printf("%d taking over\n",myid);
							//char c = getchar();
							//master(sz,argc,argv,f,myid);
							//MASTER_ID = new_master_id;
							printf("new also dead\n");
							break;
						}
						else{
							printf("\n\n================ old master dead====================\n");
							//MASTER_ID = new_master_id;
							//char c = getchar();
							start_time = MPI_Wtime();
							timeout = master_timeout*3;
							new_master_id =  myid;
							//break;
						}
					}
					//else
					//{
						// inform others
						send_all_processes(&myid,TAG_MASTER_DIED,sz,myid);
						printf(" master dead for %d\n",myid);
						INFORMED_OTHERS = TRUE;
						timeout = master_timeout*3;
						start_time = MPI_Wtime();

					//}
			}
			if(flag){
				MPI_Get_count(&status_size, MPI_BYTE, &size);
				w_work = (work_unit *)malloc(f->work_sz);
				//printf("size message to receive%d\n",size);
				unsigned char *serialized_work = (unsigned char *)malloc(sizeof(unsigned char)*size);
				// Receive chunks of work
				MPI_Recv(serialized_work,size, MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status_w);
				// if work tag received
				// Compute the results 
				if(status_w.MPI_TAG == TAG_MASTER_DIED){
					if(INFORMED_OTHERS == FALSE){
						send_all_processes(&myid,TAG_MASTER_DIED,sz,myid);
						printf(" got to know master dead %d\n",myid);
						INFORMED_OTHERS = TRUE;

					}
					timeout = master_timeout*3;
					if(new_master_id > status_w.MPI_SOURCE){
						new_master_id = status_w.MPI_SOURCE;
					}
					printf("lowest master decided by %d is %d\n", myid, new_master_id );
					MASTER_ID = new_master_id;
				}

				if(status_w.MPI_TAG == TAG_WORK){
					MASTER_ID = status_w.MPI_SOURCE;
					w_r = f->get_result_object();
					temp_result = f->get_result_object();
					printf("start deserializing\n");
					w_work = f->deserialize(serialized_work,size);
					printf("get work first after deserializing %lu\n", f->work_first(w_work));
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
							printf("last work first %d %lu\n", myid, f->work_first(w_work));
							send_heartbeat(myid, MASTER_ID);
							// printf("partially complete and work first after partial result %lu for processor %d\n",f->work_first(w_work),myid);
						}
					}
					// Send it back to the master
					// serialize result
					serialized_result = f->serialize_result(w_r,&len);

					//printf("serialized length %d\n",(int)(*serialized_result));
					//MPI_Send(serialized_result, len, MPI_BYTE, MASTER_ID, TAG_RESULT, MPI_COMM_WORLD);
					//FAIL THE WORKER
					free(w_r);
					free(temp_result);
					printf("last work first %d %lu\n", myid, f->work_first(w_work));
					//free(w_work);
					printf("sending result_unit %d\n", myid);
					F_Send(serialized_result, len, MPI_BYTE, MASTER_ID, TAG_RESULT, MPI_COMM_WORLD, myid);

				}

				// if termination tag received cleanup
				if(status_w.MPI_TAG == TAG_TERMINATE){
					free(w_work);
					break;
				}
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
   		MPI_Request request;
      	return MPI_Isend (buf, count, datatype, dest, tag, comm, &request);
   }
}

//can be implemented using the built-in random number generator and comparing the value returned to the threshold p
int random_fail(int myid){
	// get random number seeded by system time and process rank
	// source: http://scicomp.stackexchange.com/questions/1274/how-can-i-seed-a-parallel-linear-congruential-pseudo-random-number-generator-for
	srand(abs(((time(NULL)*181)*((myid-83)*359))%104729));
	int randomNum = rand() % 101;

	int p = 101;//change as needed

	if(randomNum > p)
	{
		return TRUE;
	} else{
		return FALSE;
	}
}
