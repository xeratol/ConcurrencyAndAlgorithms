#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <random>
#include <chrono>
#include <thread>
#include <future>

#include <cassert>

#define USE_PARALLEL 1
#define ENABLE_PRINT 0
#define NUM_ELEMENTS 1000000

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

std::vector<int> quickSort(std::vector<int>&& source, typename std::vector<int>::size_type startIndex, typename std::vector<int>::size_type endIndex)
{
    typedef typename std::vector<int>::size_type size_type;
    std::queue<size_type> lowHighIndices;
    lowHighIndices.push(startIndex);
    lowHighIndices.push(endIndex);

    while (!lowHighIndices.empty())
    {
        auto low = lowHighIndices.front();
        lowHighIndices.pop();
        auto high = lowHighIndices.front();
        lowHighIndices.pop();

        if (high - low <= 1)
        {
            continue;
        }

        int pivotValue = source[low];
        size_type pivotIndex = low + 1;

        // partitioning
        for (auto i = pivotIndex; i < high; ++i)
        {
            if (source[i] < pivotValue)
            {
                std::swap(source[i], source[pivotIndex]);
                ++pivotIndex;
            }
        }

        pivotIndex--;
        std::swap(source[low], source[pivotIndex]);

        lowHighIndices.push(low);
        lowHighIndices.push(pivotIndex);

        lowHighIndices.push(pivotIndex + 1);
        lowHighIndices.push(high);
    }

    return source;
}

std::vector<int> mergeSortedLists(const std::vector<int>& a, const std::vector<int>& b)
{
    std::vector<int> result;
    result.reserve(a.size() + b.size());
    auto aIndex = 0u;
    auto bIndex = 0u;

    while (aIndex < a.size() || bIndex < b.size())
    {
        if (aIndex == a.size())
        {
            result.push_back(b[bIndex]);
            bIndex++;
        }
        else if (bIndex == b.size())
        {
            result.push_back(a[aIndex]);
            aIndex++;
        }
        else if (a[aIndex] < b[bIndex])
        {
            result.push_back(a[aIndex]);
            aIndex++;
        }
        else
        {
            result.push_back(b[bIndex]);
            bIndex++;
        }
    }

    return result;
}

std::vector<int> mergeSortedLists(std::vector<std::vector<int>> sortedPartialLists)
{
    std::vector<int> result;

    if (sortedPartialLists.size() < 1)
    {
        return result;
    }

    result = sortedPartialLists[0];
    for (auto i = 1; i < sortedPartialLists.size(); ++i)
    {
        result = std::move(mergeSortedLists(result, sortedPartialLists[i]));
    }

    return result;
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
#if USE_PARALLEL
    auto numThreads = std::thread::hardware_concurrency();
    std::cout << "Num Threads: " << numThreads << std::endl;
    auto numElementsPerThread = source.size() / static_cast<float>(numThreads);
    std::cout << "Num Elements per Thread: " << numElementsPerThread << std::endl;
    auto low = source.begin();

    std::vector<std::future<std::vector<int>>> futurePartialLists;
    futurePartialLists.reserve(numThreads - 1); // main thread doesn't need a future

    for (auto i = 0u; i < numThreads - 1; ++i)
    {
        auto high = source.begin() + static_cast<int>( roundf( (i + 1) * numElementsPerThread ) );
        futurePartialLists.emplace_back(std::async([low, high]
        {
            // low and high iterators are valid because source doesn't get modified until all portions are sorted
            auto partial = std::vector<int>(low, high);
            return quickSort(std::move(partial), 0, partial.size());
        }));
        low = high;
    }

    std::vector<std::vector<int>> sortedPartialLists;
    sortedPartialLists.resize(numThreads);

    {
        // main thread does work too
        auto partial = std::vector<int>(low, source.end());
        sortedPartialLists[numThreads - 1] = quickSort(std::move(partial), 0, partial.size());
    }

    // get all sorted portions
    for (auto i = 0u; i < numThreads - 1; ++i)
    {
        sortedPartialLists[i] = futurePartialLists[i].get();
    }

#if ENABLE_PRINT
    std::for_each(sortedPartialLists.cbegin(), sortedPartialLists.cend(), [](const auto& list)
    {
        std::cout << (isSorted(list) ? "sorted" : "not sorted") << std::endl;
        print(list);
    });
#endif

    source = std::move(mergeSortedLists(std::move(sortedPartialLists)));
#else
    source = std::move(quickSort(std::move(source), 0, source.size()));
#endif

    assert(source.size() == NUM_ELEMENTS);
    auto stopTime = std::chrono::high_resolution_clock::now();

    std::cout << (isSorted(source) ? "sorted" : "not sorted") << std::endl;
#if ENABLE_PRINT
    print(source);
#endif

    std::cout << "Duration: " << std::chrono::duration<float, std::milli>(stopTime - startTime).count() << "ms" << std::endl;
    return 0;
}
