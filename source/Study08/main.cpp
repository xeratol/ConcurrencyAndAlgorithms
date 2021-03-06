#include <iostream>
#include <string>
#include <mutex>
#include <future>
#include <thread>
#include <deque>

std::mutex tasks_mutex;
std::deque<std::packaged_task<void()>> tasks;
bool worker_done = false;

void worker_thread()
{
    std::cout << "worker thread: " << std::this_thread::get_id() << std::endl;
    while (!worker_done)
    {
        std::packaged_task<void()> task;
        {
            std::lock_guard<std::mutex> lock(tasks_mutex);
            if (tasks.empty())
            {
                continue;
            }
            task = std::move(tasks.front());
            tasks.pop_front();
        }

        // execute in worker thread
        //task();

        // dispatch subthreads
        std::thread worker_subthread(std::move(task));
        worker_subthread.detach(); // safe to detach because a future will block
    }
}

template <typename Callable>
std::future<void> queue_task(Callable f)
{
    std::packaged_task<void()> task(f);
    auto return_future = task.get_future();
    std::lock_guard<std::mutex> lock(tasks_mutex);
    tasks.push_back(std::move(task));
    return return_future;
}

void print_task(const std::string& s, const int timeDelaySeconds)
{
    std::this_thread::sleep_for(std::chrono::seconds(timeDelaySeconds));
    std::cout << s << " executing in " << std::this_thread::get_id() << std::endl;
}

int main()
{
    std::cout << "main thread: " << std::this_thread::get_id() << std::endl;
    std::thread worker(worker_thread);

    auto task1_future = queue_task([] { print_task("task1", 10); });
    auto task2_future = queue_task([] { print_task("task2", 9); });
    auto task3_future = queue_task([] { print_task("task3", 8); });

    task1_future.wait();
    task2_future.wait();
    task3_future.wait();

    worker_done = true;
    worker.join();
    return 0;
}