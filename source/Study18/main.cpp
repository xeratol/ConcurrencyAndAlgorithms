#include <iostream>
#include <filesystem>

#include "EasyBMP.h"

namespace fs = std::experimental::filesystem;

int main()
{
    auto dataFolder = fs::path("../data");
    auto imagePath = dataFolder / "4x4.bmp";

    BMP image;
    if (!image.ReadFromFile(imagePath.string().c_str()))
    {
        std::cout << "Failed to open " + imagePath.string() << std::endl;
        return 1;
    }

    std::cout << "Width: " << image.TellWidth() << std::endl;
    std::cout << "Height: " << image.TellHeight() << std::endl;
    std::cout << "Bit Depth: " << image.TellBitDepth() << std::endl;

    for (auto x = 0; x < image.TellWidth(); ++x)
    {
        for (auto y = 0; y < image.TellHeight(); ++y)
        {
            auto pixel = image.GetPixel(x, y);
            auto gray = 0.3f * pixel.Red + 0.59f * pixel.Green + 0.11f * pixel.Blue;
            pixel.Red = pixel.Green = pixel.Blue = static_cast<unsigned char>(gray);
            image.SetPixel(x, y, pixel);
        }
    }

    auto outputImagePath = fs::path() / "out.bmp";
    fs::remove(outputImagePath);
    if (!image.WriteToFile(outputImagePath.string().c_str()))
    {
        std::cout << "Failed to write " + outputImagePath.string() << std::endl;
        return 1;
    }

    return 0;
}