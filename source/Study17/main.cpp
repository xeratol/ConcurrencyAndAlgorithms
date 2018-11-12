#include <iostream>
#include <string>
#include <complex>
#include <vector>
#include <limits>
#include <algorithm>

#include <cmath>

#define NDEBUG
#include <cassert>

#include "fft.h"

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

bool Test(std::vector<std::complex<double>>& in, std::vector<std::complex<double>>& expOut, bool print = false)
{
    if (print)
    {
        std::cout << "Input data:\n";
        PrintVector(in);
    }

    const auto N = RoundUpPowerOf2(static_cast<unsigned int>(in.size()));
    std::vector<std::complex<double>> d(in.size());
    const auto bitRev = GenBitReversal(N);
    for (auto i = 0u; i < N; ++i)
    {
        d[i] = in[bitRev[i]];
    }

    if (print)
    {
        std::cout << "Input data after bit-reversal:\n";
        PrintVector(d);
    }

    const auto twiddle = GenTwiddleFactors(N / 2);

    if (print)
    {
        std::cout << "Twiddle Factors:\n";
        PrintVector(twiddle);
    }

    const auto result = ApplyButterfly(d, twiddle);

    // test phase
    if (print)
    {
        std::cout << "Result:\n";
        PrintVector(result);
    }

    if (result.size() != expOut.size())
    {
        return false;
    }

    for (auto i = 0u; i < result.size(); ++i)
    {
        auto a = abs(result[i]);
        auto b = abs(expOut[i]);
        if ( !isEqual( a, b, 0.000001) )
        {
            return false;
        }
    }

    return true;
}

int main()
{
    std::vector<std::vector<std::complex<double>>> inputs
    {
        { 1, 2, 4, 4 },
        { 1, 0, 1, 0 },
        { 1, 1, 0, 0 },
        { 1, 2, 4, 4, 1, 2, 4, 4 },
        { 1, 2, 3, 4, 5, 6, 7, 8 },
    };

    std::vector<std::vector<std::complex<double>>> outputs
    {
        { 11, {-3, 2}, -1, {-3, -2} },
        { 2, 0, 2, 0 },
        { 2, {1, -1}, 0, {1, 1} },
        { 22, 0, {-6, 4}, 0, -2, 0, {-6, -4}, 0 },
        { 36, {-4, 9.65}, {-4, 4}, {-4, 1.65}, -4, {-4, -1.65}, {-4, -4}, {-4, -9.65} },
    };

    for (auto i = 0u; i < inputs.size(); ++i)
    {
        std::cout << (Test(inputs[i], outputs[i]) ? "worked" : "failed") << std::endl;
    }

    return 0;
}