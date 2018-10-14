#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <thread>

#define USE_PARALLEL 1
#define ENABLE_PRINT 0
#define NUM_ELEMENTS 1000

void populate_sorted(std::vector<int>& data, int numElements)
{
    int i = 0;
    while (i < numElements)
    {
        data.push_back(i);
        ++i;
    }
}

void populate_random(std::vector<int>& data, int numElements)
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

bool isSorted(const std::vector<int>& data)
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

void print(const std::vector<int>& data)
{
    std::for_each(data.begin(), data.end(), [](int d)
    {
        std::cout << d << " ";
    });
    std::cout << std::endl << std::endl;
}

template <class T, class size_type = typename std::vector<T>::size_type>
void quickSort(std::vector<T>& source, size_type startIndex, size_type endIndex)
{
    if (endIndex - startIndex <= 1)
    {
        return;
    }

    T pivotValue = source[startIndex];
    size_type pivotIndex = startIndex + 1;

    // partitioning
    for (auto i = pivotIndex; i < endIndex; ++i)
    {
        if (source[i] < pivotValue)
        {
            std::swap(source[i], source[pivotIndex]);
            ++pivotIndex;
        }
    }

    pivotIndex--;
    std::swap(source[startIndex], source[pivotIndex]);

    // recursion
#if USE_PARALLEL
    std::thread lower(quickSort<T>, std::ref(source), startIndex, pivotIndex);
    quickSort(source, pivotIndex + 1, endIndex);
    lower.join();
#else
    quickSort(source, startIndex, pivotIndex);
    quickSort(source, pivotIndex + 1, endIndex);
#endif
}

int main()
{
    std::vector<int> source;
    //populate_sorted(source, NUM_ELEMENTS);
    populate_random(source, NUM_ELEMENTS);

    std::cout << (isSorted(source) ? "sorted" : "not sorted") << std::endl;
#if ENABLE_PRINT
    print(source);
#endif

    auto startTime = std::chrono::high_resolution_clock::now();
    quickSort(source, (size_t)0, source.size());
    auto stopTime = std::chrono::high_resolution_clock::now();

    std::cout << (isSorted(source) ? "sorted" : "not sorted") << std::endl;
#if ENABLE_PRINT
    print(source);
#endif

    std::cout << "Duration: " << std::chrono::duration<float, std::milli>(stopTime - startTime).count() << "ms" << std::endl;
    return 0;
}
