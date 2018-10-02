# Concurrency And Algorithms

## Independent Study to learn modern C++ Multi-Threading

The purpose of this repository is for me to understand, explore, and practice C++ multi-threading
functionalities for algorithms.

The source codes and documents within this repo are created by me unless otherwise stated. They
are meant to be personal projects and not to be used for production.

## Repository Structure

- :file_folder: [**docs**](docs) contains PDF/PPTX slides of summaries, notes, etc
- :file_folder: [**source**](source) contains the Visual Studio 2017 Solution and Projects
  - :file_folder: [**common**](source/common) contains some common code shared between my case studies. This includes LodePNG, EasyBMP, and some wrappers and utilities I made
  - :file_folder: [**data**](source/data) contains the data sets I used for my experiment
    - :page_facing_up: [**sources.txt**](source/data/sources.txt) contains links to some binary data (such as images) used but are impractical to copy into the repo
  - :file_folder: **StudyXX** (where XX is a 2-digit number) contains a specific case study

## Case Studies
### Study 01 - Hello World
Used `std::thread` for a simple application

### Study 02 - Mutex and Locking
Used `std::mutex`, `std::lock_guard<std::mutex>`, `std::thread::hardware_concurrency()`, `std::this_thread::get_id()` to lock I/O for printing

### Study 03 - Passing Data by Reference
Used `std::ref` to properly pass data by reference from caller to thread

*Note: code is prone to race conditions*

### Study 04 - Parallel File Copy
Used `std::experimental::filesystem` (C++17) to transfer files in parallel. Used `std::chrono` to measure and compare performance between different number of threads

*Note: `from` folder is intentionally left empty. Put it some large files to test this case study.*

*Note: This was a failed experiment. Operation is blocked by File I/O.*

### Study 05 - Convert Image to Grayscale
Used `lodepng` to decode and encode PNG images.

## References:
- [C++ Concurrency in Action by Anthony Williams](https://www.cplusplusconcurrencyinaction.com/)
- [Effective Modern C++ by Scott Meyers](https://www.aristeia.com/EMC++.html)

## External Libraries
- [LodePNG by Lode Vandevenne](https://lodev.org/lodepng/)
- [EasyBMP by Paul Macklin](http://easybmp.sourceforge.net/)

## Resources
- [Images from NASA](https://www.nasa.gov/multimedia/imagegallery/index.html)
