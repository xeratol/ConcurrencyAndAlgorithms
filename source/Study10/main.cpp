#include <iostream>
#include <chrono>
#include <future>
#include <thread>

int producer()
{
    std::cout << "Producer Thread ID: " << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    return 1;
}

void consumer(std::shared_future<int> sharedFuture)
{
    std::cout << "Consumer (" << std::this_thread::get_id() << ") started" <<  std::endl;
    sharedFuture.wait();
    std::cout << "Consumer (" << std::this_thread::get_id() << ") done" << std::endl;
    std::cout << "Consumer (" << std::this_thread::get_id() << ") received " << sharedFuture.get() << std::endl;
}

int main()
{
    auto data_orig = std::async(producer);
    std::shared_future<int> data = data_orig.share();

    // data_orig is no longer valid when it's converted into a shared_future

    // shared_future is copied, not moved
    std::thread worker1(consumer, data);
    std::thread worker2(consumer, data);

    worker1.join();
    worker2.join();
    return 0;
}