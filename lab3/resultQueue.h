#ifndef resultQueue_h
#define resultQueue_h

typedef struct result_node_t{
   result_unit *result;
   struct result_node_t *next;
} result_node;

typedef struct result_queue_node_t
{
   /* data */
   result_node *front, *rear;
} result_queue_node, *result_queue;


result_queue queue_create(void);
int queue_empty(result_queue queue);
void dequeue(result_queue queue);
void queue_destroy(result_queue queue);
void enqueue(result_queue queue, result_unit *result);

#endif
