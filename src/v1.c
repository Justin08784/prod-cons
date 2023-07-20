#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <time.h>
#include <stdint.h>
#include "unistd.h"

#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

#include <utlist.h>
#include <utarray.h>

#define BUFFER_SIZE 32
#define ENABLE_DEBUG true


typedef struct node {
    void *data;
    size_t bytes;
    struct node *prev;
    struct node *next;
} node_t;


inline void *ck_malloc(size_t size)
{
    void *rv = malloc(size);
    if (!rv)
        exit(-1);
    return rv;
}

inline node_t *create_node(void *data, size_t bytes)
{
    node_t *rv = ck_malloc(sizeof(node_t));
    rv->data = data;
    rv->bytes = bytes;
    rv->prev = NULL;
    rv->next = NULL;
    return rv;
}

inline void free_node(node_t *node)
{
    free(node);
}


node_t *list = NULL;
sem_t full;
sem_t empty;
sem_t list_guard;



void consume(void *data)
{
    free(data);
}

void *produce(void *arg, size_t *out_bytes)
{
    static uint32_t i = 0;
    uint32_t *new_val = (uint32_t *)ck_malloc(sizeof(uint32_t));
    *new_val = i++;
    *out_bytes = sizeof(uint32_t);
    return (void *) new_val;
}

void *producer_thread(void *arg)
{
    for (;;) {
        
        size_t bytes;
        void *datum = produce(arg, &bytes);
        node_t *node = create_node(datum, bytes);

        sem_wait(&empty);
        sem_wait(&list_guard);
        DL_APPEND(list, node);
        if (ENABLE_DEBUG)
            printf("PROD %u\n", *((uint32_t *)node->data));
        sem_post(&list_guard);
        sem_post(&full);
    }

}

void *consumer_thread(void *arg)
{
    // time_t t;
    // srand((unsigned) time(&t));

    uint rand_state;
    rand_state = time(NULL) ^ getpid() ^ (uint) pthread_self();


    for (;;) {
        sem_wait(&full);
        sem_wait(&list_guard);
        node_t *head = list;
        DL_DELETE(list, head);
        if (ENABLE_DEBUG)
            printf("CONS %u\n", *((uint32_t *)head->data));
        sem_post(&list_guard);
        /* we can mark as available (i.e. increment empty) because the node 
        is taken from the list anyways */
        sem_post(&empty);
        
        usleep(rand_r(&rand_state) % 1000000);
        consume(head->data);
        free_node(head);
    }
}





int main(int argc, char *argv[]) 
{
    /* sem_open instead of sem_init should be used on macos, 
    where unnamed semaphores are not supported for some reason */
    // sem_unlink("/full_sem");
    // sem_unlink("/empty_sem");
    // sem_unlink("/list_guard");


    // full = sem_open("/full_sem", O_CREAT, 0644, 0);
    // empty = sem_open("/empty_sem", O_CREAT, 0644, 10);
    // list_guard = sem_open("/list_guard", O_CREAT, 0644, 1);
    // printf("full: %p\n", full);
    // printf("empty: %p\n", empty);
    // printf("list_guard: %p\n", list_guard);
    // assert (full != SEM_FAILED);
    // assert (empty != SEM_FAILED);
    // assert (list_guard != SEM_FAILED);


    sem_init(&full, 0, 0);
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&list_guard, 0, 1);






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

    return 0;
}