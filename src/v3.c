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

typedef enum status {
    EMPTY = 0b00,
    EMPTYING = 0b01,
    FILLING = 0b10,
    FULL = 0b11
} status_t;

#define BUFFER_SIZE 32 // in blocks
#define BLOCK_SIZE 32 // in bytes
#define ENABLE_DEBUG false

#define PROD_LIMIT 10000 // max # of production cycles
#define CONS_TIME 1000 // in microseconds
#define PROD_TIME 1000 // in microseconds

#define STATUS_BITS 2
#define STATUSES_PER_WORD 16 // __WORDSIZE/STATUS_BITS

void *buffer[BUFFER_SIZE];
uint32_t free_map[BUFFER_SIZE / STATUSES_PER_WORD + 1];
sem_t full;
sem_t empty;
sem_t buff_guard;

sem_t nval_guard;
uint32_t nval = 0;


inline void *ck_malloc(size_t size)
{
    void *rv = malloc(size);
    if (!rv)
        exit(-1);
    return rv;
}

inline int sindex(int b) { return b / STATUSES_PER_WORD; }
inline int soffset(int b) { return STATUS_BITS * (b % STATUSES_PER_WORD); }

inline void clear_status(int index) 
{ 
    uint64_t wipe_mask = ~(0b11 << soffset(index));
    free_map[sindex(index)] &= wipe_mask;
}

inline void set_status(int index, status_t val) 
{ 
    uint64_t wipe_mask = ~(0b11 << soffset(index));
    free_map[sindex(index)] &= wipe_mask;
    
    free_map[sindex(index)] |= (val << soffset(index)); 
}

inline int get_status(int index) 
{ 
    // printf("  %x\n", 0b11 << soffset(index));
    return (free_map[sindex(index)] >> soffset(index)) & 0b11;
}


inline int find_free()
{
    sem_wait(&buff_guard);
    for (size_t free_index = 0; free_index < BUFFER_SIZE; ++free_index) {
        if (get_status(free_index) == EMPTY) {
            set_status(free_index, FILLING);
            sem_post(&buff_guard);
            return free_index;
        }
    }

    printf("error: no free slots\n");
    exit(-1);    
}

inline int find_full()
{
    sem_wait(&buff_guard);
    for (size_t full_index = 0; full_index < BUFFER_SIZE; ++full_index) {
        if (get_status(full_index) == FULL) {
            set_status(full_index, EMPTYING);
            sem_post(&buff_guard);
            return full_index;
        }
    }
    
    printf("error: no full slots\n");
    exit(-1);    
}

inline void mark_as_full(int index)
{
    sem_wait(&buff_guard);
    set_status(index, FULL);
    sem_post(&buff_guard);
}

inline void mark_as_empty(int index)
{
    sem_wait(&buff_guard);
    set_status(index, EMPTY);
    sem_post(&buff_guard);
}



void consume(void *data)
{
    usleep(CONS_TIME);
    return;
}

void produce(void *arg, void *out_dst, size_t *out_bytes)
{
    sem_wait(&nval_guard);
    *((uint32_t *) out_dst) = nval++;
    sem_post(&nval_guard);
    *out_bytes = sizeof(uint32_t);
    usleep(PROD_TIME);
}

void *producer_thread(void *arg)
{
    uint32_t rand_state = time(NULL) ^ getpid() ^ pthread_self();

    for (size_t cycle_no = 0; cycle_no < PROD_LIMIT; ++cycle_no) {
        size_t bytes;

        sem_wait(&empty);

        int free_index = find_free();
        produce(NULL, buffer[free_index], &bytes);
        if (ENABLE_DEBUG)
            printf("PROD %u\n", *((uint32_t *) buffer[free_index]));
        
        mark_as_full(free_index);
        sem_post(&full);
    }

    return NULL;

}

void *consumer_thread(void *arg)
{
    uint32_t rand_state = time(NULL) ^ getpid() ^ pthread_self();


    for (;;) {
        sem_wait(&full);

        int full_index = find_full();
        void *val = buffer[full_index];
        if (ENABLE_DEBUG)
            printf("CONS %u\n", *((uint32_t *) val));
        consume(val);

        mark_as_empty(full_index);
        sem_post(&empty);
    }
}



int main(int argc, char *argv[]) 
{
    // for (size_t i = 0; i < 128; ++i) {
    //     printf("i: %lu, sin: %d, sof: %d\n", i, sindex(i), soffset(i));
    //     uint64_t wipe_mask = ~(0b11 << soffset(i));
    //     uint64_t select_mask = 0b11 << soffset(i);
    //     printf("  sele_mask: %x\n", select_mask);
    //     printf("  wipe_mask: %x\n", wipe_mask);
    //     printf("  sum: %x\n", select_mask + wipe_mask);
        
    // }

    // printf("%x\n", free_map[0]);
    // for (size_t i = 0; i < 16; ++i) {
    //     set_status(i, FULL);
    //     printf("set %x\n", free_map[0]);
    //     assert(get_status(i) == FULL);
    //     if (i > 0) {
    //         set_status(i - 1, EMPTY);
    //         printf("cle %x\n", free_map[0]);
    //         assert(get_status(i - 1) == EMPTY);
    //     }
    // }
    // return 0;

    for (size_t i = 0; i < BUFFER_SIZE; ++i)
        buffer[i] = ck_malloc(BLOCK_SIZE);

    sem_init(&full, 0, 0);
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&buff_guard, 0, 1);
    sem_init(&nval_guard, 0, 1);

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

    /* wait for all producers to exit, then count number of empty blocks */
    for (size_t i = 0; i < BUFFER_SIZE; ++i)
        sem_wait(&empty);
    /* if all buffer blocks are empty, all consumers must have finished */
    return 0;
}