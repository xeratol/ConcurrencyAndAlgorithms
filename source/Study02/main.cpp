#include <iostream>
#include <thread>
#include <vector>
#include <algorithm>
#include <mutex>

std::mutex cout_mutex;

void print(int val)
{
    std::lock_guard<std::mutex> guard(cout_mutex);
    std::cout << "Thread " << std::this_thread::get_id() << ": " << val << std::endl;
}

void foo(int num)
{
    for (auto i = 0; i < 10; ++i)
    {
        print(num + i);
    }
}

int main()
{
    const auto numHardwareThreads = std::thread::hardware_concurrency();
    std::cout << "Hardware Threads: " << numHardwareThreads << std::endl;

    auto threads = std::vector<std::thread>();
    for (auto j = 0; j < numHardwareThreads; ++j)
    {
        threads.emplace_back(foo, j * 10);
    }

    std::for_each(threads.begin(), threads.end(), [](std::thread& t)
    {
        t.join();
    });

    return 0;
}