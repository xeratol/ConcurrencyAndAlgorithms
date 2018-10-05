#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

auto a = 0;
std::mutex data_mutex;
std::condition_variable data_cond;
auto producer_sleep_duration = 3ll;

void producer_thread()
{
    std::cout << "producer sleep" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(producer_sleep_duration));
    std::cout << "producer wakes up" << std::endl;
    std::lock_guard<std::mutex> producer_lock(data_mutex);
    ++a;
    data_cond.notify_one();
    std::cout << "producer done" << std::endl;
}

void consumer_thread()
{
    std::unique_lock<std::mutex> consumer_lock(data_mutex); // needs to be unique_lock to be able to unlock
    data_cond.wait(consumer_lock, // lock mutex and check co-supplied function
        // if function returns true, keep lock and proceed
        // if function returns false, unlock and wait
    [] {
        std::cout << "consumer checking" << std::endl;
        return a > 0;
    });
    auto b = a;                     // copy shared data locally
    consumer_lock.unlock();         // in order to unlock mutex asap
    std::cout << "consumer done";
}

int main()
{
    std::thread producer(producer_thread);
    std::thread consumer(consumer_thread);

    producer.join();
    consumer.join();

    return 0;
}