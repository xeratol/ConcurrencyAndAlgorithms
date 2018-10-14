#include <iostream>
#include <list>
#include <future>
#include <chrono>

/* Note: Run in Debug with No Optimization to see the difference */

#define ENABLE_PRINT 0
#define NUM_ELEMENTS 10000
#define NUM_MOVES 10000

void populate_sorted(std::list<int>& data, int numElements)
{
    int i = 0;
    while (i < numElements)
    {
        data.push_back(i);
        ++i;
    }
}

void print(const std::list<int>& data)
{
#if !ENABLE_PRINT
    return;
#endif
    std::for_each(data.begin(), data.end(), [](int d)
    {
        std::cout << d << " ";
    });
    std::cout << std::endl << std::endl;
}

int main()
{
    std::list<int> source;
    populate_sorted(source, NUM_ELEMENTS);
    print(source);

    auto startTime = std::chrono::high_resolution_clock::now();
    for (auto i = 0; i < NUM_MOVES; ++i)
    {
        std::list<int> temp = std::move(source);
        source = std::move(temp);
    }
    auto stopTime = std::chrono::high_resolution_clock::now();
    std::cout << "Single Thread: " << std::chrono::duration<float, std::milli>(stopTime - startTime).count() << "ms" << std::endl;
    print(source);

    startTime = std::chrono::high_resolution_clock::now();
    for (auto i = 0; i < NUM_MOVES; ++i)
    {
        std::future<std::list<int>> tempFuture = std::async([s = std::move(source)] { return s; });
        source = std::move(tempFuture.get());
    }
    stopTime = std::chrono::high_resolution_clock::now();

    std::cout << "Double Threads: " << std::chrono::duration<float, std::milli>(stopTime - startTime).count() << "ms" << std::endl;
    print(source);

    return 0;
}