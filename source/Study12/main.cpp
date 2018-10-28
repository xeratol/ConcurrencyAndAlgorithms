#include <iostream>
#include <future>
#include <vector>

const auto numIncrementsPerThread = 100000ul;
auto a = 0ull;

void synchronizing_method()
{
    std::cout << "synchronizer " << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

void racer(std::shared_future<void> synchronizer)
{
    std::cout << "racer " << std::this_thread::get_id() << std::endl;
    synchronizer.wait();

    for (auto i = 0ul; i < numIncrementsPerThread; ++i)
    {
        ++a; // race condition
    }
}

int main()
{
    auto synchronizer = std::async(synchronizing_method).share();

    std::thread worker1(racer, synchronizer);
    std::thread worker2(racer, synchronizer);

    worker1.join();
    worker2.join();

    std::cout << a << std::endl;

    return 0;
}