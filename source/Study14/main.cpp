#include <iostream>
#include <list>
#include <future>
#include <chrono>

void populate_sorted(std::list<int>& data, int numElements)
{
    int i = 0;
    while (i < numElements)
    {
        data.push_back(i);
        ++i;
    }
}

int main()
{
    std::list<int> source;
    std::list<int> temp;
    populate_sorted(source, 1000);

    auto startTime = std::chrono::high_resolution_clock::now();
    for (auto i = 0; i < 1000; ++i)
    {
        temp = std::move(source);
        source = std::move(temp);
    }
    auto stopTime = std::chrono::high_resolution_clock::now();
    std::cout << "Duration: " << std::chrono::duration<float, std::milli>(stopTime - startTime).count() << "ms" << std::endl;

    std::future<std::list<int>> tempFuture;
    startTime = std::chrono::high_resolution_clock::now();
    for (auto i = 0; i < 1000; ++i)
    {
        tempFuture = std::async([s = std::move(source)] { return s; });
        source = std::move(tempFuture.get());
    }
    stopTime = std::chrono::high_resolution_clock::now();

    std::cout << "Duration: " << std::chrono::duration<float, std::milli>(stopTime - startTime).count() << "ms" << std::endl;

    return 0;
}