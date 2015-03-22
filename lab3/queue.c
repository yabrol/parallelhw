#include "mw_api.h"
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TRUE 1
#define FALSE 0

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

void enqueue(work_queue queue, work_unit *work, int id)
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
  new_work->id = id;
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
