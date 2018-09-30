#include <iostream>
#include <fstream>
#include <string>
#include <filesystem> // C++17
#include <chrono>
#include <vector>
#include <thread>
#include <algorithm>
#include <cmath>

#define USE_THREADS 1

namespace fs = std::experimental::filesystem;

unsigned int GetOptimalNumberOfThreads(const unsigned int numTasks)
{
    auto hardwareThreadCount = std::thread::hardware_concurrency();
    if (hardwareThreadCount == 0)
    {
        hardwareThreadCount = 2;
    }
    return (numTasks > hardwareThreadCount) ? hardwareThreadCount : numTasks;
}

void CleanDestinationFolder(const fs::path& path)
{
    std::cout << "Clearing destination folder " << fs::absolute(path) << std::endl;
    fs::remove_all(path);
    fs::create_directory(path);
}

std::vector<fs::path> GetListOfFiles(const fs::path& path)
{
    std::cout << "Getting list of files from " << fs::absolute(path) << std::endl;
    auto list = std::vector<fs::path>();
    for (auto &dirItem : fs::directory_iterator(path))
    {
        if (fs::is_regular_file(dirItem))
        {
            list.push_back(dirItem);
        }
    }
    std::cout << list.size() << " files found" << std::endl;
    return list;
}

void CopyFile(const fs::path& src, const fs::path& dst)
{
    std::cout << "Copying " << src << " to " << dst << std::endl;
    fs::copy_file(src, dst);
}

void CopyFiles(const std::vector<fs::path>& files, const fs::path& dst)
{
#if USE_THREADS
    const auto fileCount = static_cast<unsigned int>(files.size());
    const auto numThreads = GetOptimalNumberOfThreads(fileCount);

    std::cout << "Number of threads: " << numThreads << std::endl;
    auto threads = std::vector<std::thread>();
    threads.reserve(numThreads - 1); // -1 because we're not counting the main thread

    auto first = files.begin();
    const auto numFilesPerThread = fileCount / static_cast<float>(numThreads);
    std::cout << "Number of files per thread: " << numFilesPerThread << std::endl;
    for (auto i = 0u; i < numThreads - 1; ++i)
    {
        const auto increment = static_cast<int>(roundf((i + 1) * numFilesPerThread));
        const auto last = files.begin() + increment;
        const auto partialFileList = std::vector<fs::path>(first, last);
        threads.emplace_back([partialFileList, &dst]
        {
            std::for_each(partialFileList.begin(), partialFileList.end(), [&dst](const fs::path& file)
            {
                CopyFile(file, dst / file.filename());
            });
        });
        first = last;
    }

    // main thread also does work
    std::for_each(first, files.end(), [&dst](const fs::path& file)
    {
        CopyFile(file, dst / file.filename());
    });

    std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));
#else
    for (auto &file : files)
    {
        CopyFile(file, dst / file.filename());
    }
#endif
}

int main()
{
    const auto destPath = fs::path("to");
    const auto sourcePath = fs::path("from");

    CleanDestinationFolder(destPath);
    auto fileList = GetListOfFiles(sourcePath);

    const auto timeStart = std::chrono::high_resolution_clock::now();
    CopyFiles(fileList, destPath);
    const auto timeEnd = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(timeEnd - timeStart);
    std::cout << "\n\nTotal transfer time: " << duration.count() * 0.000001f << "s" << std::endl;

    return 0;
}