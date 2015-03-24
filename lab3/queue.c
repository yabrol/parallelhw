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
		return;// NULL;
	}
	if(queue_empty(queue) == TRUE){
		printf("Queue Empty");
		return;// NULL;
	}
	else{
		//check for one element
		if(queue->front == queue->rear){
			work_node *temp = queue->front;
      //work_node *temp = get_work_node(queue->front->work,queue->front->id);
      queue->front = NULL;
			queue->rear = NULL;
      //temp->next = NULL;
			free(temp);
      return;// temp;
		}
		else{
      work_node *temp = queue->front;
			queue->front = (queue->front)->next;
      temp->next = NULL;
      free(temp);
      return;// temp;
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

work_node *get_work_node(work_unit *work, int id){
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
  return new_work;
}

work_node *dequeue_by_id(work_queue queue, int id){
  if(queue == NULL){
    printf("Queue Pointer Empty");
    return NULL;
  }
  if(queue_empty(queue) == TRUE){
    printf("Queue Empty");
    return NULL;
  }
  else{
    work_node *ptr = queue->front;
    work_node *prev = NULL;
    while(ptr!=NULL){
      if(ptr->id == id){
          //check for one element
          if(queue->front == queue->rear){
            work_node *temp = get_work_node(queue->front->work,queue->front->id);
            queue->front = NULL;
            queue->rear = NULL;
            //temp->next = NULL;
            return temp;
          }
          else{
            if(ptr == queue->front){
              work_node *temp;
              temp = queue->front;
              queue->front = queue->front->next;
              temp->next = NULL;
              return temp;
            }
            else if(ptr == queue->rear)
            {
              printf("element at rear\n");
              work_node *temp = ptr;
              queue->rear = prev;
              queue->rear->next = NULL;
              temp->next = NULL;
              return temp;
            }
            else{
              work_node *temp = ptr;
              prev->next = ptr->next;
              temp->next = NULL;
              return temp;
            }
          } 
      }
      else{
        prev = ptr;
        ptr = ptr->next;
      }
    }
    return prev;
  }
}

void enqueue(work_queue queue, work_node *n_work)
{
  work_node *new_work;

  /* Allocate space for a node in the linked list. */
  new_work = (work_node *)malloc(sizeof(work_node));
  new_work = n_work;
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

void print_queue(work_queue queue){
work_node* temp = queue->front;
  while(temp){
    printf("work id %d on queue\n", temp->id);
    temp = temp->next;
  }
}