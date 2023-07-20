#include <iostream>
#include <queue>
#include <condition_variable>
#include <thread>
#include <mutex>
#include <string>
#include <semaphore>



// std::binary_semaphore hi;
std::counting_semaphore full, empty;
std::queue<void *> ready;
std::mutex ready_guard;


void *producer_thread(void *arg)
{
    while (true) {
        ready.pop();
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