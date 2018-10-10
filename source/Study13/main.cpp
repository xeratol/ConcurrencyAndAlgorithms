#include <iostream>
#include <list>
#include <algorithm>
#include <random>
#include <chrono>
#include <memory>

#include <cassert>

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

std::list<int> qsort(std::list<int> &&source)
{
    std::list<int> result;
    if (source.size() <= 1)
    {
        result.splice(result.end(), source);
        return result;
    }

    int pivotValue = source.front();
    source.pop_front(); // memory deallocation

    //auto lower = std::make_unique<std::list<int>>();
    std::list<int> lower;
    //auto higher = std::make_unique<std::list<int>>();
    std::list<int> higher;

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

    if (lower.size() > 0)
    {
        result.splice(result.end(), qsort(std::move(lower)));
    }

    result.push_back(pivotValue); // memory allocation

    if (higher.size() > 0)
    {
        result.splice(result.end(), qsort(std::move(higher)));
    }

    return result;
}

int main()
{
    std::list<int> source;
    populate_random(source, 1000);

    std::cout << (isSorted(source) ? "sorted" : "not sorted") << std::endl;
    //print(source);

    auto startTime = std::chrono::high_resolution_clock::now();
    auto final = qsort(std::move(source));
    auto stopTime = std::chrono::high_resolution_clock::now();

    std::cout << (isSorted(final) ? "sorted" : "not sorted") << std::endl;
    //print(source);

    std::cout << "Duration: " << std::chrono::duration<float, std::milli>(stopTime - startTime).count() << "ms" << std::endl;
    return 0;
}
