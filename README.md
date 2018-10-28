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
### [Study 01 - Hello World](source/Study01)
Used `std::thread` for a simple application

### [Study 02 - Mutex and Locking](source/Study02)
Used `std::mutex`, `std::lock_guard<std::mutex>`, `std::thread::hardware_concurrency()`, `std::this_thread::get_id()` to lock I/O for printing

### [Study 03 - Passing Data by Reference](source/Study03)
Used `std::ref` to properly pass data by reference from caller to thread

*Note: code is prone to race conditions*

### [Study 04 - Parallel File Copy](source/Study04)
Used `std::experimental::filesystem` (C++17) to transfer files in parallel. Used `std::chrono` to measure and compare performance between different number of threads

*Note: `from` folder is intentionally left empty. Put it some large files to test this case study.*

*Note: This was a failed experiment. Operation is blocked by File I/O.*

### [Study 05 - Convert Image to Grayscale](source/Study05)
Used `lodepng` to decode and encode PNG images.

### [Study 06 - Synchronizing Threads](source/Study06)
Used `std::condition_variable`

### [Study 07 - Threads with return values](source/Study07)
Used `std::future` with `std::async`

### [Study 08 - Passing Functions to Threads](source/Study08)
Used `std::packaged_task`
Passing arbitrary work for threads to execute
Demonstrated a dedicated worker thread that does arbitrary work

### [Study 09 - Threads setting values before terminating](source/Study09)
Used `std::promise`

### [Study 10 - Passing the same data to multiple threads](source/Study10)
Used `std::shared_future`

### [Study 11 - Timeouts of Futures](source/Study11)

### [Study 12 - Forcing Problematic Race Conditions](source/Study12)

### [Study 13 - Parallel Quicksort 1](source/Study13)
Spawning a thread for every lower portion
Moves list data between threads
Unregulated number of threads

*Note: This is a failed experiment. The parallel version took longer than the serial version.*

### [Study 14 - Cost of moving data between threads](source/Study14)
Demonstration on the time cost of (moving) copying data between threads

*Note: Moving one list to another only incurs moving the head pointer. Moving a list to a thread is a deep copy.*

### [Study 15 - Parallel Quicksort 2](source/Study15)
Spawning a thread for every lower portion
References vector data between threads
Unregulated number of threads

*Note: This is a failed experiment. The parallel version took longer than the serial version.*

### [Study 16 - Parallel Sort](source/Study16)
Spawns a fixed number of threads
Divides total number of elements evenly amongst threads
Moves vecor data to and from threads restricted only at the beginning and at the end of each thread's lifetime
Each thread performs quicksort and returns their sorted portion
Main thread performs merge sort on all the sorted portions

## References:
- [C++ Concurrency in Action by Anthony Williams](https://www.cplusplusconcurrencyinaction.com/)
- [Effective Modern C++ by Scott Meyers](https://www.aristeia.com/EMC++.html)

## External Libraries
- [LodePNG by Lode Vandevenne](https://lodev.org/lodepng/)
- [EasyBMP by Paul Macklin](http://easybmp.sourceforge.net/)

## Resources
- [Images from NASA](https://www.nasa.gov/multimedia/imagegallery/index.html)
