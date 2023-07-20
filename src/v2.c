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
#define BLOCKS_PER_WORD 32 / 2
#define FREE_MAP_SIZE BUFFER_SIZE / BLOCKS_PER_WORD


typedef enum status {
    EMPTY = 0b00,
    FULL = 0b01,
    EMPTYING = 0b10,
    FILLING = 0b11
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


node_t buffer[BUFFER_SIZE];
uint32_t free_map[FREE_MAP_SIZE];
sem_t full;
sem_t empty;
sem_t buff_guard;

inline size_t bindex(size_t b) { return b / BLOCKS_PER_WORD; }
inline size_t boffset(size_t b) { return 2 * (b % BLOCKS_PER_WORD); }

inline void set_bit(int b) 
{ 
    free_map[bindex(b)] |= (0b11 << boffset(b)); 
}
inline void clear_bit(int b) 
{ 
    free_map[bindex(b)] &= ~(0b11 << boffset(b));
}
inline int get_bit(int b) 
{ 
    return free_map[bindex(b)] & (0b11 << boffset(b));
}

inline node_t *find_free()
{
    sem_wait(&buff_guard);
    size_t free_index = 0;
    for (;;) {
        if (free_index >= BUFFER_SIZE)
            exit(-1);
        
        free_map[]
        
    }    

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
        sem_wait(&buff_guard);
        // DL_APPEND(buffer, node);
        if (ENABLE_DEBUG)
            printf("PROD %u\n", *((uint32_t *)node->data));
        sem_post(&buff_guard);
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
        sem_wait(&buff_guard);
        node_t *head = buffer;
        // DL_DELETE(buffer, head);
        if (ENABLE_DEBUG)
            printf("CONS %u\n", *((uint32_t *)head->data));
        sem_post(&buff_guard);
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
   
    assert((BUFFER_SIZE & 0b11) == 0); // BUFFER_SIZE should be a multiple of 4
    for (size_t i = 0; i < FREE_MAP_SIZE; ++i)
        free_map[i] = 0;
    

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


    sem_init(&full, 0, 0);
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&buff_guard, 0, 1);






    size_t num_producers = 1;
    size_t num_consumers = 0;

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