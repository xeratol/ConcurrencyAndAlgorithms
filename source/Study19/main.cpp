#include <iostream>
#include <filesystem>

#include "EasyBMP.h"
#include "matrix.h"

#define USE_THREADS 1

#if USE_THREADS
#include "pfft.h"
#else
#include "fft.h"
#endif

namespace fs = std::experimental::filesystem;

// Multiply elements with (-1)^(x + y) or
// Negate elements where (x + y) is odd
void RecenterMatrixDataForFFT(matrix<std::complex<double>>& data)
{
    for (auto y = 0ull; y < data.Height(); ++y)
    {
        for (auto x = 0ull; x < data.Width(); ++x)
        {
            if (((x + y) & 1) == 1)
            {
                data.At(x, y) = -data.At(x, y);
            }
        }
    }
}

BMP ConvertFromMatrix(const matrix<double>& data)
{
    BMP image;
    image.SetSize(static_cast<int>(data.Width()),
        static_cast<int>(data.Height()));

    for (auto y = 0; y < data.Height(); ++y)
    {
        for (auto x = 0; x < data.Width(); ++x)
        {
            const auto val = data.At(x, y) * 255.0;
            const auto gray = static_cast<unsigned char>(val);
            image.SetPixel(x, y, { gray, gray, gray, 255 });
        }
    }

    return image;
}

/*
 Returned matrix has real from [0, 1] and imag 0
*/
matrix<std::complex<double>> ConvertToMatrix(const BMP& image)
{
    matrix<std::complex<double>> result(image.TellWidth(), image.TellHeight());

    for (auto y = 0; y < image.TellHeight(); ++y)
    {
        for (auto x = 0; x < image.TellWidth(); ++x)
        {
            const auto pixel = image.GetPixel(x, y);
            //auto gray = 0.3f * pixel.Red + 0.59f * pixel.Green + 0.11f * pixel.Blue;
            const auto gray = (static_cast<double>(pixel.Red) + 
                static_cast<double>(pixel.Green) + 
                static_cast<double>(pixel.Blue)) / 3.0;
            result.At(x, y).real(gray / 255.0);
        }
    }

    return result;
}

matrix<std::complex<double>> GetMatrixFromImage(const fs::path& path)
{
    BMP image;
    if (!image.ReadFromFile(path.string().c_str()))
    {
        std::cout << "Failed to open " + path.string() << std::endl;
        return matrix<std::complex<double>>();
    }
    return ConvertToMatrix(image);
}

matrix<double> GetMagnitudeSpectrum(const matrix<std::complex<double>>& data)
{
    std::vector<double> magSpecData(data.Height() * data.Width());
    data.Transform([](const std::complex<double>& el) { return abs(el); }, magSpecData.begin());
    return matrix<double>(data.Width(), data.Height(), std::move(magSpecData));
}

void CleanUpDataForImageWriting(matrix<double>& data)
{
    data.Transform([](const double& el)
    {
        return log(el * 1000.0 + 1)/log(1000.0 + 1);
    });

    data.Normalize();
}

void WriteBMPToFile(BMP& image, const fs::path& path)
{
    auto outputImagePath = fs::path() / path;
    fs::remove(outputImagePath);
    if (!image.WriteToFile(outputImagePath.string().c_str()))
    {
        std::cout << "Failed to write " + outputImagePath.string() << std::endl;
    }
}

void WriteMatrixToImage(const matrix<std::complex<double>>& data, const std::string& filename, bool cleanUp = true)
{
    auto magMatrix = GetMagnitudeSpectrum(data);
    if (cleanUp)
    {
        CleanUpDataForImageWriting(magMatrix);
    }
    auto magImage = ConvertFromMatrix(magMatrix);
    WriteBMPToFile(magImage, fs::path() / filename);
}

int main()
{
    auto imageMatrix = GetMatrixFromImage(fs::path("../data/square-256.bmp"));
    //auto imageMatrix = GetMatrixFromImage(fs::path("../data/dummy.bmp"));
    //auto imageMatrix = GetMatrixFromImage(fs::path("../data/line-256-2.bmp"));
    const auto MN = imageMatrix.Raw().size();
    RecenterMatrixDataForFFT(imageMatrix);

    matrix<std::complex<double>> intermediate;
#if USE_THREADS
    imageMatrix = PFFT(imageMatrix, intermediate);
#else
    imageMatrix = FFT(imageMatrix, intermediate);
#endif

    WriteMatrixToImage(intermediate, "fwd-byRow.bmp");
    WriteMatrixToImage(imageMatrix, "fwd-out.bmp");

    imageMatrix.Transform([](const std::complex<double>& d) { return std::conj(d); });
#if USE_THREADS
    imageMatrix = PFFT(imageMatrix, intermediate);
#else
    imageMatrix = FFT(imageMatrix, intermediate);
#endif

    RecenterMatrixDataForFFT(intermediate);
    intermediate.Transform([&](const std::complex<double>& d)
    {
        return std::conj(d) / static_cast<double>(MN);
    });
    WriteMatrixToImage(intermediate, "inv-byRow.bmp", true);

    RecenterMatrixDataForFFT(imageMatrix);
    imageMatrix.Transform([&](const std::complex<double>& d)
    {
        return std::conj(d) / static_cast<double>(MN);
    });

    WriteMatrixToImage(imageMatrix, "inv-out.bmp", false);

    return 0;
}