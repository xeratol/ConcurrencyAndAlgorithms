#include <iostream>
#include <thread>
#include <vector>
#include <algorithm>

void increment(int a) // try removing &
{
    ++a;
}

int main()
{
    int a = 0;
    auto threads = std::vector<std::thread>();
    for (auto i = 0; i < 10; ++i)
    {
        threads.emplace_back(increment, std::ref(a)); // try removing std::ref
    }

    std::for_each(threads.begin(), threads.end(), [](std::thread& t)
    {
        t.join();
    });

    std::cout << a << std::endl;

    return 0;
}