#include <future>

#include "fft.h"
#include "pfft.h"

matrix<std::complex<double>> PFFT(const matrix<std::complex<double>>& data,
    matrix<std::complex<double>>& intermediate)
{
    matrix<std::complex<double>> dataExtended(data);
    const auto N = RoundUpPowerOf2(static_cast<unsigned int>(data.Width()));
    const auto M = RoundUpPowerOf2(static_cast<unsigned int>(data.Height()));
    dataExtended.Resize(N, M);

    const auto bitRevN = GenBitReversal(N);
    const auto bitRevM = GenBitReversal(M);
    const auto twiddleN = GenTwiddleFactors(N / 2);
    const auto twiddleM = GenTwiddleFactors(M / 2);

    const auto numThreads = std::thread::hardware_concurrency();
    std::vector<std::future<matrix<std::complex<double>>>> threads(numThreads - 1);

    {
        const auto numTasksPerThread = M / static_cast<float>(numThreads - 1); // main thread won't work
        auto first = 0u;
        for (auto i = 0u; i < numThreads - 1; ++i)
        {
            const auto last = static_cast<unsigned int>(round(numTasksPerThread * (i + 1)));
            const auto range = last - first;
            matrix<std::complex<double>> partialData(N, range);
            for (auto j = 0u; j < range; ++j)
            {
                partialData.Row(j, dataExtended.Row(first + j));
            }
            first = last;

            threads[i] = std::async([=]
            {
                matrix<std::complex<double>> partialIntermediate(N, range);
                for (auto y = 0u; y < range; ++y)
                {
                    auto rowData = partialData.Row(y);
                    partialIntermediate.Row(y, FFT(rowData, bitRevN, twiddleN));
                }
                return partialIntermediate;
            });
        }
    }

    intermediate.Resize(N, M);
    {
        auto lastRowAppended = 0u;
        for (auto i = 0u; i < numThreads - 1; ++i)
        {
            auto partialData = threads[i].get();
            for (auto j = 0; j < partialData.Height(); ++j)
            {
                intermediate.Row(lastRowAppended, partialData.Row(j));
                ++lastRowAppended;
            }
        }
    }

    {
        const auto numTasksPerThread = N / static_cast<float>(numThreads - 1); // main thread won't work
        auto first = 0u;
        for (auto i = 0u; i < numThreads - 1; ++i)
        {
            const auto last = static_cast<unsigned int>(round(numTasksPerThread * (i + 1)));
            const auto range = last - first;
            matrix<std::complex<double>> partialData(range, M);
            for (auto j = 0u; j < range; ++j)
            {
                partialData.Col(j, intermediate.Col(first + j));
            }
            first = last;

            threads[i] = std::async([=]
            {
                matrix<std::complex<double>> partialIntermediate(range, M);
                for (auto x = 0u; x < range; ++x)
                {
                    auto colData = partialData.Col(x);
                    partialIntermediate.Col(x, FFT(colData, bitRevM, twiddleM));
                }
                return partialIntermediate;
            });
        }
    }

    matrix<std::complex<double>> result(N, M);
    {
        auto lastColAppended = 0u;
        for (auto i = 0u; i < numThreads - 1; ++i)
        {
            auto partialData = threads[i].get();
            for (auto j = 0; j < partialData.Width(); ++j)
            {
                result.Col(lastColAppended, partialData.Col(j));
                ++lastColAppended;
            }
        }
    }

    return result;
}