#include "mw_api.h"
#include "resultQueue.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TRUE 1
#define FALSE 0

result_queue rQueue_create(void)
{
  result_queue queue;
  queue = (result_queue)malloc(sizeof(result_queue_node));
  if (queue == NULL) {
    fprintf(stderr, "Insufficient memory for new queue.\n");
    exit(1);  /* Exit program, returning error code. */
  }
  queue->front = queue->rear = NULL;
  return queue;
}

int rQueue_empty(result_queue queue){
	if(queue == NULL)
		return TRUE;
	if(queue->front == NULL)
		return TRUE;
	return FALSE;
}

result_node *rDequeue(result_queue queue)
{
	if(queue == NULL){
		printf("Queue Pointer Empty");
		return NULL;
	}
	if(rQueue_empty(queue) == TRUE){
		printf("Queue Empty");
		return NULL;
	}
	else{
		//check for one element
		if(queue->front == queue->rear){
      result_node *temp = queue->front;
			queue->front = NULL;
			queue->rear = NULL;
      temp->next = NULL;
			return temp;
		}
		else{
      result_node *temp = queue->front;
			queue->front = (queue->front)->next;
      temp->next = NULL;
      return temp;
		}
	}

}

void rQueue_destroy(result_queue queue)
{
  /*
   * First remove each element from the queue (each
   * element is in a dynamically-allocated node.)
   */
  while (!rQueue_empty(queue))
    rDequeue(queue);

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

result_node *rGet_result_node(result_unit *result, int id){
  result_node *new_result;

  /* Allocate space for a node in the linked list. */
  new_result = (result_node *)malloc(sizeof(result_node));
  if (new_result == NULL) {
    fprintf(stderr, "Insufficient memory for new queue element.\n");
    exit(1);  /* Exit program, returning error code. */
  }
  /* Place information in the node */
  new_result->result = result;
  new_result->id = id;
  new_result->next = NULL;
  return new_result;
}

result_node *rDequeue_by_id(result_queue queue, int id){
  if(queue == NULL){
    printf("Queue Pointer Empty");
    return NULL;
  }
  if(rQueue_empty(queue) == TRUE){
    printf("Queue Empty");
    return NULL;
  }
  else{
    //check for one element
    result_node *ptr = queue->front;
    result_node *prev = NULL;
    while(ptr!=NULL){
      if(ptr->id == id){
          //check for one element
          if(queue->front == queue->rear){
            result_node *temp = queue->front;
            queue->front = NULL;
            queue->rear = NULL;
            temp->next = NULL;
            return temp;
          }
          else{
            if(ptr == queue->front){
              result_node *temp;
              temp = queue->front;
              queue->front = queue->front->next;
              temp->next = NULL;
              return temp;
            }
            else if(ptr == queue->rear)
            {
              result_node *temp = ptr;
              queue->rear = prev;
              prev->next = NULL;
              temp->next = NULL;
              return temp;
            }
            else{
              result_node *temp = ptr;
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

void rEnqueue(result_queue queue, result_node *new_result)
{
  /*
   * Link the element into the right place in
   * the linked list.
   */
  if (queue->front == NULL) {  /* Queue is empty */
    queue->front = queue->rear = new_result;
  }
  else {
    queue->rear->next = new_result;
    queue->rear = new_result;
  }
}
