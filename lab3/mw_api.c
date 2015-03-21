#include "mw_api.h"
#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <time.h>

#define TRUE 1
#define FALSE 0
#define MASTER_ID 0
#define TAG_WORK 0
#define TAG_RESULT 1
#define TAG_TERMINATE 99

int random_fail();

typedef struct work_node_t{
	work_unit *work;
	struct work_node_t *next;
} work_node;

typedef struct work_queue_node_t
{
	/* data */
	work_node *front, *rear;
} work_queue_node, *work_queue;


work_queue queue_create(void)
{
  work_queue queue;
  queue = (work_queue)malloc(sizeof(work_queue_node));
  if (queue == NULL) {
    fprintf(stderr, "Insufficient memory for new queue.\n");
    exit(1);  /* Exit program, returning error code. */
  }
  queue->front = queue->rear = NULL;
  return queue;
}

int queue_empty(work_queue queue){
	if(queue == NULL)
		return TRUE;
	if(queue->front == NULL)
		return TRUE;
	return FALSE;
}

void dequeue(work_queue queue)
{
	if(queue == NULL){
		printf("Queue Pointer Empty");
		return;
	}
	if(queue_empty(queue) == TRUE){
		printf("Queue Empty");
		return;
	}
	else{
		//check for one element
		if(queue->front == queue->rear){
			queue->front = NULL;
			queue->rear = NULL;
			return;
		}
		else{
			queue->front = (queue->front)->next;
		}
	}

}

void queue_destroy(work_queue queue)
{
  /*
   * First remove each element from the queue (each
   * element is in a dynamically-allocated node.)
   */
  while (!queue_empty(queue))
    dequeue(queue);

  /*
   * Reset the front and rear just in case someone
   * tries to use them after the CDT is freed.
   */
  queue->front = queue->rear = NULL;

  /*
   * Now free the structure that holds information
   * about the queue.
   */
  free(queue);
}

void enqueue(work_queue queue, work_unit *work)
{
  work_node *new_work;

  /* Allocate space for a node in the linked list. */
  new_work = (work_node *)malloc(sizeof(work_node));
  if (new_work == NULL) {
    fprintf(stderr, "Insufficient memory for new queue element.\n");
    exit(1);  /* Exit program, returning error code. */
  }
  /* Place information in the node */
  new_work->work = work;
  new_work->next = NULL;
  /*
   * Link the element into the right place in
   * the linked list.
   */
  if (queue->front == NULL) {  /* Queue is empty */
    queue->front = queue->rear = new_work;
  }
  else {
    queue->rear->next = new_work;
    queue->rear = new_work;
  }
}

void send_work(int wid,work_queue wq,struct mw_api_spec *f){
	int size;
	work_unit *chunk = (work_unit *)malloc(f->work_sz);
	chunk = wq->front->work;
	unsigned char *serialized_chunk = f->serialize(chunk,&size);
	//printf("Serializing done %lu\n",(int)(*serialized_chunk));
	MPI_Send(serialized_chunk, size, MPI_CHAR, wid, TAG_WORK, MPI_COMM_WORLD );
	//printf("Process %d out of %d\n", wid, sz);
	//wid = 1 + (wid)%(sz-1);
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
		work_unit **work;// = (work_unit *)malloc(f->work_sz);
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
		//printf("total_workers %d\n",sz-1);
		// Wait for the results
		result_unit **results = (result_unit **)malloc((n_chunks)*sizeof(result_unit *));
		i=0;
		for(wid=0;wid<sz;wid++){
			
			result_unit *r = (result_unit *)malloc(f->res_sz);
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
			i++;
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
      return MPI_Send (buf, count, datatype, dest, tag, comm);
   }
}

//can be implemented using the built-in random number generator and comparing the value returned to the threshold p
int random_fail(){
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

