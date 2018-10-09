#include <iostream>
#include <future>
#include <vector>

auto a = 0;

int synchronizing_method()
{
    std::cout << "synchronizer " << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return 1;
}

void racer(std::shared_future<int> synchronizer)
{
    std::cout << "racer " << std::this_thread::get_id() << std::endl;
    synchronizer.wait();
    ++a; // race condition
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