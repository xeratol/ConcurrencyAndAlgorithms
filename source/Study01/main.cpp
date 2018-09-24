#include <iostream>
#include <thread>                   // C++ Standard for Threads

void hello()                        // main() of the other thread
{
    std::cout << "Hello world!\n";
}

int main(int argc, char** argv)
{
    std::thread t(hello);           // start new thread
    t.join();                       // wait
    return 0;
}
