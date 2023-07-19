#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <time.h>

#include <pthread.h>
#include <semaphore.h>

#include <utlist.h>
#include <utarray.h>




typedef struct node {
  void *data;
  size_t bytes;
  struct node *prev;
  struct node *next;
} node_t;


void *ck_malloc(size_t size, char *info_str)
{
    void *rv = (void *)malloc(size);
    if (!rv) {
        printf("Failed to realloc in %s.\n", info_str);
        exit(1);
    }
    return rv;
}

node_t *create_node(void *data, size_t bytes)
{
  node_t *rv = ck_malloc(sizeof(node_t), "create_node()");
  rv->data = data;
  rv->bytes = bytes;
  rv->prev = NULL;
  rv->next = NULL;
  return rv;
}

// void print_queue(queue_t *queue)
// {
//   assert(queue);

//   printf("(");
//   for (node_t *curr = queue->list; curr; curr = curr->next) {
//     assert(curr->bytes == sizeof(uint32_t));
//     printf("%u ", *((uint32_t *) curr->data));
//   }
//   printf(")\n");
// }


node_t *list = NULL;
sem_t *full = NULL;
sem_t *empty = NULL;
sem_t *list_guard = NULL;


pthread_mutex_t *guard;

uint32_t arr[10];

void *producer_thread(void *arg)
{
  size_t i = 0;
  int out;
  sem_getvalue(empty, &out);
  printf("out: %d\n", out);
  while (true) {
    i = (i + 1) % 10;

    node_t *node = create_node(&arr[i], sizeof(uint32_t));
    // printf("A\n");
    sem_wait(empty);
    sem_wait(list_guard);
    DL_APPEND(list, node);
    printf("PROD %u\n", *((uint32_t *)node->data));
    sem_post(list_guard);
    sem_post(full);
  }

}

void *consumer_thread(void *arg)
{
  while (true) {
    // printf("B\n");
    sem_wait(full);
    sem_wait(list_guard);
    node_t *head = list;
    DL_DELETE(list, head);
    printf("CONS %u\n", *((uint32_t *)head->data));
    free(head);
    sem_post(list_guard);
    sem_post(empty);
  }
}





int main(int argc, char *argv[]) 
{
//   full = sem_open("/full_sem/a", O_CREAT, 0644, 0);
//   empty = sem_open("/empty_sem/b", O_CREAT, 0644, 10);
//   list_guard = sem_open("/list_guard/c", O_CREAT, 0644, 1);
  sem_unlink("/full_sem");
  sem_unlink("/empty_sem");
  sem_unlink("/list_guard");


  full = sem_open("/full_sem", O_CREAT, 0644, 0);
  empty = sem_open("/empty_sem", O_CREAT, 0644, 10);
  list_guard = sem_open("/list_guard", O_CREAT, 0644, 1);
  printf("full: %p\n", full);
  printf("empty: %p\n", empty);
  printf("list_guard: %p\n", list_guard);
  assert (full != SEM_FAILED);
  assert (empty != SEM_FAILED);
  assert (list_guard != SEM_FAILED);

  time_t t;
  srand((unsigned) time(&t));



  node_t *curr;
  int count;
  for (size_t i = 0; i < 10; ++i)
      arr[i] = i;
  DL_COUNT(list, curr, count);
  printf("count  %u\n", count);
  DL_FOREACH(list, curr) printf("%u ", *((uint32_t *) curr->data));


  size_t num_producers = 10;
  size_t num_consumers = 10;

  pthread_t producers[num_producers];
  pthread_t consumers[num_consumers];
  for (size_t i = 0; i < num_producers; ++i)
    pthread_create(&producers[i], NULL, producer_thread, NULL);
  
  for (size_t i = 0; i < num_consumers; ++i)
    pthread_create(&consumers[i], NULL, consumer_thread, NULL);
  
  for (size_t i = 0; i < num_producers; ++i)
    pthread_join(producers[i], NULL);
  for (size_t i = 0; i < num_consumers; ++i)
    pthread_join(consumers[i], NULL);

  


  // pthread_t producer_id, consumer_id, cc;
  // pthread_create(&producer_id, NULL, producer_thread, NULL);
  // pthread_create(&consumer_id, NULL, consumer_thread, NULL);
  // pthread_create(&cc, NULL, consumer_thread, NULL);

  // pthread_join(producer_id, NULL);
  // pthread_join(consumer_id, NULL);
  // pthread_join(cc, NULL);





  // node_t *curr;
  // int count;
  // for (size_t i = 0; i < 10; ++i) {
  //     arr[i] = i;
  //     curr = create_node(&arr[i], sizeof(uint32_t));
  //     DL_APPEND(list, curr);
  // }
  // DL_COUNT(list, curr, count);
  // printf("count  %u\n", count);
  // DL_FOREACH(list, curr) printf("%u ", *((uint32_t *) curr->data));


  

  
  

  

  return 0;
}