#include <iostream>
#include <queue>
#include <condition_variable>
#include <thread>
#include <mutex>
#include <string>
#include <semaphore>
#include <cassert>



#define MAX_EMPTY 1024

std::counting_semaphore<MAX_EMPTY> full{0}, empty{MAX_EMPTY};
std::queue<void *> ready;
std::mutex ready_guard;

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

        Producer() {}

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

        Consumer() {}

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
    size_t num_consumers = 4;

    Producer producers[num_producers];
    Consumer consumers[num_consumers];

    for (size_t i = 0; i < num_producers; ++i) {
        producers[i] = Producer(&ready, &full, &empty, &ready_guard, produce);
        producers[i].run();
    }

    for (size_t i = 0; i < num_consumers; ++i) {
        consumers[i] = Consumer(&ready, &full, &empty, &ready_guard, consume);
        consumers[i].run();
    }

    for (size_t i = 0; i < num_producers; ++i)
        producers[i].thread.join();

    for (size_t i = 0; i < num_consumers; ++i)
        consumers[i].thread.join();
}


