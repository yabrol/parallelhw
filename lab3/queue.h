#ifndef queue_h
#define queue_h

typedef struct work_node_t{
   work_unit *work;
   int id;
   struct work_node_t *next;
} work_node;

typedef struct work_queue_node_t
{
   /* data */
   work_node *front, *rear;
} work_queue_node, *work_queue;


work_queue queue_create(void);
int queue_empty(work_queue queue);
void dequeue(work_queue queue);
void queue_destroy(work_queue queue);
void enqueue(work_queue queue, work_node *new_work);
work_node *get_work_node(work_unit *work, int id);
work_node *dequeue_by_id(work_queue queue, int id);
void print_queue(work_queue queue);

#endif
