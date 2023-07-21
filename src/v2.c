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

#define BUFFER_SIZE 16
#define ENABLE_DEBUG true
#define BLOCKS_PER_WORD 16
#define FREE_MAP_SIZE 1


typedef enum status {
    EMPTY = 0b00,
    EMPTYING = 0b01,
    FILLING = 0b10,
    FULL = 0b11
} status_t;


typedef struct node {
    void *data;
    size_t bytes;
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
    return rv;
}

inline void free_node(node_t *node)
{
    free(node);
}


node_t *buffer[BUFFER_SIZE];
status_t free_map[BUFFER_SIZE];
sem_t full;
sem_t empty;
sem_t buff_guard;


int find_free()
{
    sem_wait(&buff_guard);
    for (size_t free_index = 0; free_index < BUFFER_SIZE; ++free_index) {
        if (free_map[free_index] == EMPTY) {
            free_map[free_index] = FILLING;
            // printf("%x\n", free_map[0]);
            sem_post(&buff_guard);
            return free_index;
        }
    }

    printf("error: no free slots\n");
    exit(-1);    
}

int find_full()
{
    sem_wait(&buff_guard);
    for (size_t full_index = 0; full_index < BUFFER_SIZE; ++full_index) {
        if (free_map[full_index] == FULL) {
            free_map[full_index] = EMPTYING;
            // printf("%x\n", free_map[0]);
            sem_post(&buff_guard);
            return full_index;
        }
    }
    
    printf("error: no full slots\n");
    exit(-1);    
}

int mark_as_full(int index)
{
    sem_wait(&buff_guard);
    free_map[index] = FULL;
    sem_post(&buff_guard);
}

int mark_as_empty(int index)
{
    sem_wait(&buff_guard);
    free_map[index] = EMPTY;
    sem_post(&buff_guard);
}



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
        int free_index = find_free();
        // printf("free %d\n", free_index);

        // DL_APPEND(buffer, node);
        if (ENABLE_DEBUG)
            printf("PROD %u\n", *((uint32_t *)node->data));

        buffer[free_index] = node;
        mark_as_full(free_index);

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
        // printf("ohno: %x\n", free_map[0]);
        int full_index = find_full();

        // printf("full %d\n", full_index);

        // DL_DELETE(buffer, head);
        /* we can mark as available (i.e. increment empty) because the node 
        is taken from the list anyways */
        node_t *datum = buffer[full_index];
        if (ENABLE_DEBUG)
            printf("CONS %u\n", *((uint32_t *)datum->data));
        mark_as_empty(full_index);
        sem_post(&empty);
        
        // usleep(rand_r(&rand_state) % 1000000);
        consume(datum->data);
        free_node(datum);
    }
}



// void tester(uint32_t x)
// {
//     printf("%u: (in: %u, of: %u)\n", x, bindex(x), boffset(x));
// }

int main(int argc, char *argv[]) 
{

    // free_map[0] = 0xfffffffc;

    // printf("free_map: %x\n", free_map[0]);

    // find_full();
    // return 0;

    sem_init(&full, 0, 0);
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&buff_guard, 0, 1);

    // tester(0);
    // tester(1);
    // tester(2);
    // for (size_t i = 0; i < 32; i++)
    //     tester(i);

    
    assert((BUFFER_SIZE & 0b11) == 0); // BUFFER_SIZE should be a multiple of 4

    // int inder = find_free();
    // printf("inder: %d\n", inder);
    // mark_as_full(inder);
    // printf("fullio: %d\n", find_full());
    
    // return 0;

    /* sem_open instead of sem_init should be used on macos, 
    where unnamed semaphores are not supported for some reason */
    // sem_unlink("/full_sem");
    // sem_unlink("/empty_sem");
    // sem_unlink("/buff_guard");


    // full = sem_open("/full_sem", O_CREAT, 0644, 0);
    // empty = sem_open("/empty_sem", O_CREAT, 0644, 10);
    // buff_guard = sem_open("/buff_guard", O_CREAT, 0644, 1);
    // printf("full: %p\n", full);
    // printf("empty: %p\n", empty);
    // printf("buff_guard: %p\n", buff_guard);
    // assert (full != SEM_FAILED);
    // assert (empty != SEM_FAILED);
    // assert (buff_guard != SEM_FAILED);









    size_t num_producers = 1;
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