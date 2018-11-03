#include <iostream>
#include <string>
#include <complex>
#include <vector>
#include <limits>
#include <algorithm>

#include <cmath>

#define NDEBUG
#include <cassert>

#include "lodepng.h"

const auto PI = 3.14159265359;

template <typename T>
bool isEqual(T a, T b)
{
    T diff = a - b;
    return (diff < std::numeric_limits<T>::epsilon() &&
        -diff < std::numeric_limits<T>::epsilon());
}

template <typename T>
void PrintVector(const std::vector<T>& data, const std::string& delimiter = "\n") {
    auto i = 0u;
    std::for_each(data.begin(), data.end(), [&](const T& d)
    {
        std::cout << "  [" << i << "]: " << d << delimiter;
        ++i;
    });
    std::cout << std::endl;
}

template <typename T>
T RoundUpPowerOf2(T v)
{
    T ans = 1;
    auto numOnes = 0u;
    while (v)
    {
        if (v & 1)
        {
            ++numOnes;
        }
        v >>= 1;
        ans <<= 1;
    }
    if (numOnes < 2)
    {
        ans >>= 1;
    }
    return ans;
}

std::vector<unsigned int> GenBitReversal(unsigned int size)
{
    std::vector<unsigned int> allReversed(size);
    const auto logOfSize = log2(size);
    assert(isEqual(floor(logOfSize), logOfSize));
    const auto numBits = static_cast<unsigned int>(logOfSize);

    for (auto i = 0u; i < size; ++i)
    {
        auto orig = i;
        auto reversed = 0u;
        for (auto j = 0u; j < numBits; ++j)
        {
            reversed <<= 1;
            if (orig & 1)
            {
                reversed |= 1;
            }
            orig >>= 1;
        }
        allReversed[i] = reversed;
    }

    return allReversed;
}

std::vector<std::complex<double>> GenTwiddleFactors(unsigned int halfN)
{
    std::vector<std::complex<double>> twiddle(halfN);

    for (auto i = 0u; i < halfN; ++i) {
        twiddle[i] = std::polar(1.0, -static_cast<float>(i) * (PI / halfN));
    }

    return twiddle;
}

std::vector<std::complex<double>> ApplyButterfly(const std::vector<std::complex<double>>& data,
                                                 const std::vector<std::complex<double>>& twiddle)
{
    assert(data.size() / 2 == twiddle.size());
    const auto N = data.size();
    std::vector<std::complex<double>> retVal(data);
    for (auto numElements = 2u; numElements <= N; numElements *= 2) {
        std::cout << "Num Elements: " << numElements << std::endl;
        for (auto offset = 0u; offset < N; offset += numElements) {
            std::cout << "Offset: " << offset << std::endl;

            // copy data to temp first
            const std::vector<std::complex<double>> temp(retVal.begin() + offset,
                retVal.begin() + offset + numElements);

            unsigned int halfNumElements = numElements / 2;
            for (auto i = 0u; i < halfNumElements; ++i) {
                std::cout << "  [" << i << "]: " << temp[i] << " + "
                    << temp[i + halfNumElements] << " * " << twiddle[i * N / numElements] << std::endl;
                retVal[i + offset] = temp[i] + 
                    temp[i + halfNumElements] * twiddle[i * N / numElements];
                std::cout << "  [" << i + halfNumElements << "]: " << temp[i] << " - "
                    << temp[i + halfNumElements] << " * " << twiddle[i * N / numElements] << std::endl;
                retVal[i + offset + halfNumElements] = temp[i] -
                    temp[i + halfNumElements] * twiddle[i * N / numElements];
            }
        }
    }
    std::cout << std::endl;
    return retVal;
}

int main()
{
    std::vector<std::complex<double>> data{1,2,4,4};
    auto N = RoundUpPowerOf2(static_cast<unsigned int>(data.size()));
    auto halfN = N / 2;

    std::cout << "Input data:\n";
    PrintVector(data);

    auto bitRev = GenBitReversal(N);
    for (auto i = 0u; i < halfN; ++i)
    {
        if (i != bitRev[i])
        {
            std::swap(data[i], data[bitRev[i]]);
        }
    }

    std::cout << "Input data after bit-reversal:\n";
    PrintVector(data);

    auto twiddle = GenTwiddleFactors(halfN);
    std::cout << "Twiddle Factors:\n";
    PrintVector(twiddle);

    auto result = ApplyButterfly(data, twiddle);

    std::cout << "Result:\n";
    PrintVector(result);

    return 0;
}