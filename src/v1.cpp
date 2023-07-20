#include <iostream>
#include <queue>
#include <condition_variable>
#include <thread>
#include <mutex>
#include <string>
#include <semaphore>
#include <cassert>



const size_t max_empty = 10;
std::counting_semaphore full{0}, empty{max_empty};
std::queue<void *> ready;
std::mutex ready_guard;


void *producer_thread(void *arg)
{
    uint32_t i = 0;
    while (true) {

        // printf("A\n");
        empty.acquire();
        uint32_t *new_val = (uint32_t *)malloc(sizeof(int));
        assert(new_val);
        *new_val = i++;

        ready_guard.lock();
        ready.push(new_val);
        assert(*((uint32_t *) ready.back()) == *new_val);
        ready_guard.unlock();
        printf("PROD %u\n", *new_val);

        full.release();
    }

}

void *consumer_thread(void *arg)
{
    while (true) {

        // printf("A\n");
        full.acquire();

        ready_guard.lock();
        uint32_t *popped_val = (uint32_t *)ready.front();
        ready.pop();
        ready_guard.unlock();
        printf("CONS %u\n", *popped_val);

        empty.release();
        
    }

}

int main(int argc, char *argv[]) 
{
    std::thread p1(producer_thread, nullptr);
    std::thread c1(consumer_thread, nullptr);
    std::thread c2(consumer_thread, nullptr);

    p1.join();
    c1.join();
    c2.join();
}


