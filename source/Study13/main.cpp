#include <iostream>
#include <list>
#include <algorithm>
#include <random>
#include <chrono>
#include <future>
#include <memory>

#include <cassert>

#define USE_PARALLEL 0

void populate_sorted(std::list<int>& data, int numElements)
{
    int i = 0;
    while (i < numElements)
    {
        data.push_back(i);
        ++i;
    }
}

void populate_random(std::list<int>& data, int numElements)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, numElements);
    while (numElements > 0)
    {
        data.push_back(dis(gen));
        --numElements;
    }
}

bool isSorted(const std::list<int>& data)
{
    auto prev = data.begin();
    auto next = prev;
    ++next;

    while (next != data.end())
    {
        if (*prev > *next)
        {
            return false;
        }

        prev = next;
        ++next;
    }
    return true;
}

void print(const std::list<int>& data)
{
    std::for_each(data.begin(), data.end(), [](int d)
    {
        std::cout << d << " ";
    });
    std::cout << std::endl << std::endl;
}

template <typename T>
std::list<T> quickSort(std::list<T>&& source)
{
    if (source.size() <= 1)
    {
        return source;
    }

    int pivotValue = source.front();
    source.pop_front(); // memory deallocation

    std::list<T> lower;
    std::list<T> higher;

    // partitioning
    while (!source.empty())
    {
        auto it = source.begin();
        if (*it < pivotValue)
        {
            lower.splice(lower.end(), source, it);
        }
        else
        {
            higher.splice(higher.end(), source, it);
        }
    }

    assert(source.size() == 0);

    // recursion
#if USE_PARALLEL
    std::future<std::list<T>> newLower = std::async(quickSort<T>, std::move(lower));
#else
    lower = quickSort(std::move(lower));
#endif
    higher = quickSort(std::move(higher));

    // merging
    std::list<T> result;
    result.push_back(pivotValue); // memory allocation
    result.splice(result.end(), higher);
#if USE_PARALLEL
    result.splice(result.begin(), newLower.get());
#else
    result.splice(result.begin(), lower);
#endif

    return result;
}

int main()
{
    const int numElements = 1000;
    std::list<int> source;
    //populate_sorted(source, numElements);
    populate_random(source, numElements);

    std::cout << (isSorted(source) ? "sorted" : "not sorted") << std::endl;
    if (numElements <= 20)
    {
        print(source);
    }

    auto startTime = std::chrono::high_resolution_clock::now();
    source = quickSort(std::move(source));
    auto stopTime = std::chrono::high_resolution_clock::now();

    std::cout << (isSorted(source) ? "sorted" : "not sorted") << std::endl;
    if (numElements <= 20)
    {
        print(source);
    }

    std::cout << "Duration: " << std::chrono::duration<float, std::milli>(stopTime - startTime).count() << "ms" << std::endl;
    return 0;
}
