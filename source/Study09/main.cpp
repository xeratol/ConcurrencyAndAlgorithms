#include <iostream>
#include <future>
#include <chrono>
#include <thread>

void worker_thread(std::promise<int> data_promise)
{
    std::cout << "Worker Thread ID: " << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout << "Worker Thread wakes" << std::endl;
    data_promise.set_value(1);
    std::cout << "Worker Thread done" << std::endl;
}

int main()
{
    std::cout << "Main Thread ID: " << std::this_thread::get_id() << std::endl;
    std::promise<int> data;
    std::future<int> data_future = data.get_future();
    std::cout << "Main Thread starts worker thread" << std::endl;
    std::thread worker(worker_thread, std::move(data));
    std::cout << "Main Thread waits" << std::endl;
    data_future.get();
    std::cout << "done" << std::endl;
    worker.join();
    return 0;
}