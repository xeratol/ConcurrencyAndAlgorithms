#include <iostream>
#include <future>
#include <chrono>

const auto sleepMs = 1000;
const auto timeoutMs = 1500;

int main()
{
    std::future<int> data = std::async([]
    {
        std::cout << "Async Thread ID: " << std::this_thread::get_id() << std::endl;
        const auto workerThreadSleep = std::chrono::milliseconds(sleepMs);
        std::this_thread::sleep_for(workerThreadSleep);
        //std::this_thread::sleep_until(std::chrono::system_clock::now() + workerThreadSleep);
        std::cout << "Async Thread done" << std::endl;
        return 1;
    });

    std::cout << "Main Thread ID: " << std::this_thread::get_id() << std::endl;
    const auto timeoutDuration = std::chrono::duration<int, std::milli>(timeoutMs);
    if (data.wait_for(timeoutDuration) == std::future_status::ready)
    {
        std::cout << "Main Thread done" << std::endl;
    }
    else
    {
        std::cout << "Main Thread timeout" << std::endl;
    }

    return 0;
}