#ifndef resultQueue_h
#define resultQueue_h

typedef struct result_node_t{
   result_unit *result;
   int id;
   struct result_node_t *next;
} result_node;

typedef struct result_queue_node_t
{
   /* data */
   result_node *front, *rear;
} result_queue_node, *result_queue;


result_queue rQueue_create(void);
int rQueue_empty(result_queue queue);
result_node *rDequeue(result_queue queue);
void rQueue_destroy(result_queue queue);
void rEnqueue(result_queue queue, result_node *new_result);
result_node *rGet_result_node(result_unit *result, int id);
result_node *rDequeue_by_id(result_queue queue, int id);

#endif