#include <iostream>
#include <future>
#include <thread>
#include <chrono>

// in milliseconds
const int asyncSleep = 1000;
const int mainSleep = 500;

int asnyc_func()
{
    std::cout << "Async Thread ID: " << std::this_thread::get_id() << std::endl;
    std::cout << "Async Thread sleeps" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(asyncSleep));
    std::cout << "Async Thread wakes" << std::endl;
    return 1;
}

int main()
{
    std::cout << "Main Thread ID: " << std::this_thread::get_id() << std::endl;
    auto data = std::async(asnyc_func);
    std::cout << "Main Thread sleeps" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(mainSleep));
    std::cout << "Main Thread wakes" << std::endl;
    data.get();
    std::cout << "data received" << std::endl;
    return 0;
}