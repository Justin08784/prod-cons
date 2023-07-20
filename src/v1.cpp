#include <iostream>
#include <queue>
#include <condition_variable>
#include <thread>
#include <mutex>
#include <string>
#include <semaphore>
#include <cassert>



#define MAX_EMPTY 10

std::counting_semaphore<MAX_EMPTY> full{0}, empty{MAX_EMPTY};
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

void *produce(void *arg)
{
    static uint32_t i = 0;
    uint32_t *new_val = (uint32_t *)malloc(sizeof(uint32_t));
    *new_val = i++;
    return (void *) new_val;
}

void *consume(void *data)
{
    free(data);
    return nullptr;
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

class Producer {
    std::queue<void *> *data;
    std::counting_semaphore<MAX_EMPTY> *full, *empty;
    std::mutex *data_guard;
    void * (*produce)(void *arg);

    void *producer_main(void *arg)
    {
        while (true) {

            empty->acquire();
            void *new_val = produce(nullptr);
            data_guard->lock();
            data->push(new_val);
            data_guard->unlock();

            printf("PROD %u\n", *((uint32_t *)new_val));

            full->release();
        }

    }
    
    public:
        std::thread thread;

        Producer(std::queue<void *> *datar, 
                 std::counting_semaphore<MAX_EMPTY> *fullr, 
                 std::counting_semaphore<MAX_EMPTY> *emptyr,
                 std::mutex *data_guardr,
                 void *(*callback_function)(void *)) 
        {
            data = datar;
            full = fullr;
            empty = emptyr;
            data_guard = data_guardr;
            produce = callback_function;

        }

        void run()
        {
            thread = std::thread(&Producer::producer_main, this, nullptr);
        }

};

class Consumer {
    std::queue<void *> *data;
    std::counting_semaphore<MAX_EMPTY> *full, *empty;
    std::mutex *data_guard;
    void * (*consume)(void *arg);

    void *consumer_main(void *arg)
    {
        while (true) {

            // printf("A\n");
            full->acquire();

            data_guard->lock();
            uint32_t *popped_val = (uint32_t *)ready.front();
            data->pop();
            data_guard->unlock();
            printf("CONS %u\n", *popped_val);
            consume((void *)popped_val);
            empty->release();            
        }

    }
    
    public:
        std::thread thread;

        Consumer(std::queue<void *> *datar, 
                 std::counting_semaphore<MAX_EMPTY> *fullr, 
                 std::counting_semaphore<MAX_EMPTY> *emptyr,
                 std::mutex *data_guardr,
                 void *(*callback_function)(void *)) 
        {
            data = datar;
            full = fullr;
            empty = emptyr;
            data_guard = data_guardr;
            consume = callback_function;

        }

        void run()
        {
            thread = std::thread(&Consumer::consumer_main, this, nullptr);
        }

};




int main(int argc, char *argv[]) 
{
    // std::thread p1(producer_thread, nullptr);
    size_t num_producers = 1;
    size_t num_consumers = 10;

    std::queue<Producer> producers;


    Producer p1(&ready, &full, &empty, &ready_guard, produce);
    p1.run();

    // std::thread c1(consumer_thread, nullptr);
    // std::thread c2(consumer_thread, nullptr);

    Consumer c1(&ready, &full, &empty, &ready_guard, consume);    
    Consumer c2(&ready, &full, &empty, &ready_guard, consume);    
    c1.run();
    c2.run();    


    p1.thread.join();
    printf("I have contracted stupidity\n");
    c1.thread.join();
    c2.thread.join();
}


