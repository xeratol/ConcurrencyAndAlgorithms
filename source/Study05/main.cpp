#include <iostream>
#include <filesystem> // C++17
#include <string>
#include <chrono>
#include <thread>
#include <algorithm>
#include <cmath>

#include "lodepng.h"

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

void ConvertToGrayscale(std::vector<unsigned char>& image, const unsigned int width,
    const unsigned int startHeight, const unsigned int endHeight)
{
    for (auto x = 0ul; x < width; ++x)
    {
        for (auto y = startHeight; y < endHeight; ++y)
        {
            auto index = 4 * (x + y * width);
            auto r = &image[index];
            auto g = &image[index + 1];
            auto b = &image[index + 2];
            //auto a = image[index + 3];

            auto gray = 0.3f * (*r) + 0.59f * (*g) + 0.11 * (*b);
            *r = *g = *b = static_cast<unsigned int>(gray);
        }
    }
}

int main()
{
    auto dataFolder = fs::path("../data");
    auto imagePath = dataFolder / "small-satellite.png";
    //std::cout << fs::absolute(imagePath) << std::endl << fs::exists(imagePath) << std::endl;

    auto image = std::vector<unsigned char>();
    auto width = 0u;
    auto height = 0u;

    auto error = lodepng::decode(image, width, height, imagePath.string());
    if (error != 0)
    {
        std::cout << "lodepng::decode error: " << lodepng_error_text(error) << std::endl;
    }
    // image contains RGBARGBARGBA...

    std::cout << "Image: " << imagePath << std::endl
        << "Width: " << width << std::endl
        << "Height: " << height << std::endl;

    const auto timeStart = std::chrono::high_resolution_clock::now();
#if USE_THREADS
    auto numThreads = GetOptimalNumberOfThreads(height);

    std::cout << "Number of threads: " << numThreads << std::endl;
    auto threads = std::vector<std::thread>();
    threads.reserve(numThreads - 1); // -1 because we're not counting the main thread

    const auto numRowsPerThread = height / static_cast<float>(numThreads);
    std::cout << "Number of rows per thread: " << numRowsPerThread << std::endl;
    auto first = 0u;
    for (auto i = 0u; i < numThreads - 1; ++i)
    {
        const auto last = static_cast<unsigned int>(roundf((i + 1) * numRowsPerThread));
        threads.emplace_back(ConvertToGrayscale, std::ref(image), width, first, last);
        first = last;
    }

    ConvertToGrayscale(image, width, first, height);

    std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));
#else
    ConvertToGrayscale(image, width, 0, height);
#endif
    const auto timeEnd = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(timeEnd - timeStart);
    std::cout << "Duration: " << duration.count() * 0.000001f << "s" << std::endl;

    auto outputImagePath = fs::path() / "out.png";
    fs::remove(outputImagePath);
    error = lodepng::encode(outputImagePath.string(), image, width, height);
    if (error != 0)
    {
        std::cout << "lodepng::encode error: " << lodepng_error_text(error) << std::endl;
    }

    return 0;
}