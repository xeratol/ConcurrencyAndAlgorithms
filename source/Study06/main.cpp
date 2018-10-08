#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

auto data = 0;
std::mutex data_mutex;
std::condition_variable data_cond;
auto producer_sleep_duration = 3ll;

void producer_thread()
{
    std::cout << "producer sleep" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(producer_sleep_duration));
    std::cout << "producer wakes up" << std::endl;
    std::lock_guard<std::mutex> producer_lock(data_mutex);
    ++data;
    data_cond.notify_one(); // or notify_all()
    std::cout << "producer done" << std::endl;
}

void consumer_thread()
{
    std::unique_lock<std::mutex> consumer_lock(data_mutex); // needs to be unique_lock to be able to unlock
    data_cond.wait(consumer_lock, // lock mutex and check co-supplied function
        // if function returns true; keep lock and proceed
        // if function returns false; block the thread, unlock mutex and try again later
    [] {
        // this function could be called multiple times to check
        // if it returns true, with or without notification
        std::cout << "consumer checking" << std::endl;
        return data > 0;
    });
    auto thread_local_data = data;  // copy shared data locally
    consumer_lock.unlock();         // unlock mutex asap
    std::cout << "consumer done";   // do something with local copy of data here
}

int main()
{
    std::thread producer(producer_thread);
    std::thread consumer(consumer_thread);

    producer.join();
    consumer.join();

    return 0;
}