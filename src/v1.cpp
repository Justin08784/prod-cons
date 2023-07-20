#include <iostream>
#include <queue>
#include <condition_variable>
#include <thread>
#include <mutex>
#include <string>
#include <semaphore>



// std::binary_semaphore hi;
const size_t max_empty = 10;
std::counting_semaphore full{0}, empty{max_empty};
std::queue<void *> ready;
std::mutex ready_guard;


void *producer_thread(void *arg)
{
    uint32_t i = 0;
    while (true) {

        ready.pop();

        // printf("A\n");
        empty.acquire();
        uint32_t *new_val = (uint32_t *)malloc(sizeof(int));
        assert(new_val);
        *new_val = i++;

        ready_guard.lock();
        ready.push(new_val);
        assert(*((uint32_t *) ready.back()) == *new_val);
        printf("PROD %u\n", *new_val);
        ready_guard.unlock();

        full.release();
    }

}

void *consumer_thread(void *arg)
{
    while (true) {
        
    }

}

int main(int argc, char *argv[]) 
{
    std::counting_semaphore hi;
}


