#include <algorithm>

#include "fft.h"

const auto PI = 3.14159265359;

bool isEqual(double a, double b, double epsilon)
{
    double diff = a - b;
    return (diff < epsilon && -diff < epsilon);
}

unsigned int RoundUpPowerOf2(unsigned int v)
{
    unsigned int ans = 1;
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
    const auto logOfSize = log2(size);
    std::vector<unsigned int> allReversed(size);
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
    for (auto i = 0u; i < halfN; ++i)
    {
        twiddle[i] = std::polar(1.0, -static_cast<double>(i) * PI / halfN);
    }
    return twiddle;
}

std::vector<std::complex<double>> ApplyButterfly(const std::vector<std::complex<double>>& data,
    const std::vector<std::complex<double>>& twiddle)
{
    const auto N = data.size();
    std::vector<std::complex<double>> retVal(data);
    for (auto numElements = 2u; numElements <= N; numElements *= 2)
    {
        //std::cout << "Num Elements: " << numElements << std::endl;
        for (auto offset = 0u; offset < N; offset += numElements)
        {
            //std::cout << "Offset: " << offset << std::endl;

            // copy data to temp first
            const std::vector<std::complex<double>> temp(retVal.begin() + offset,
                retVal.begin() + offset + numElements);

            unsigned int halfNumElements = numElements / 2;
            for (auto i = 0u; i < halfNumElements; ++i)
            {
                //std::cout << "  [" << i << "]: "                   << temp[i] << " + " << temp[i + halfNumElements] << " * " << twiddle[i * N / numElements] << std::endl;
                retVal[i + offset]                   = temp[i] + temp[i + halfNumElements] * twiddle[i * N / numElements];
                //std::cout << "  [" << i + halfNumElements << "]: " << temp[i] << " - " << temp[i + halfNumElements] << " * " << twiddle[i * N / numElements] << std::endl;
                retVal[i + offset + halfNumElements] = temp[i] - temp[i + halfNumElements] * twiddle[i * N / numElements];
            }
        }
    }
    //std::cout << std::endl;
    return retVal;
}

/// <summary>
/// Returns a Fourier-Transformed data, possibly resize to the next power of 2
/// </summary>
std::vector<std::complex<double>> FFT(const std::vector<std::complex<double>>& data,
    const std::vector<unsigned int>& bitRev,
    const std::vector<std::complex<double>>& twiddle)
{
    const auto N = RoundUpPowerOf2(static_cast<unsigned int>(data.size()));

    if (twiddle.size() * 2 != N)
    {
        throw std::exception("Invalid Twiddle Factors");
    }

    if (bitRev.size() != N)
    {
        throw std::exception("Invalid Bit Reversal Indices");
    }

    if (data.size() < N)
    {
        throw std::exception("data is not a power of 2");
    }

    std::vector<std::complex<double>> d(data.size());
    for (auto i = 0u; i < N; ++i)
    {
        d[i] = data[bitRev[i]];
    }

    return ApplyButterfly(d, twiddle);
}

matrix<std::complex<double>> FFT(const matrix<std::complex<double>>& data, matrix<std::complex<double>>& intermediate)
{
    matrix<std::complex<double>> dataExtended(data);
    const auto N = RoundUpPowerOf2(static_cast<unsigned int>(data.Width()));
    const auto M = RoundUpPowerOf2(static_cast<unsigned int>(data.Height()));
    dataExtended.Resize(N, M);

    const auto bitRevN = GenBitReversal(N);
    const auto bitRevM = GenBitReversal(M);
    const auto twiddleN = GenTwiddleFactors(N / 2);
    const auto twiddleM = GenTwiddleFactors(M / 2);

    intermediate.Resize(N, M);
    for (auto y = 0u; y < M; ++y)
    {
        auto rowData = dataExtended.Row(y);
        intermediate.Row(y, FFT(rowData, bitRevN, twiddleN));
    }

    matrix<std::complex<double>> result(N, M);
    for (auto x = 0u; x < N; ++x)
    {
        auto colData = intermediate.Col(x);
        result.Col(x, FFT(colData, bitRevM, twiddleM));
    }

    return result;
}
