
#include <thread>
#include <iostream>
#include "mylib/mylib.hpp"


int main()
{

    using std::reference_wrapper;

    std::vector<int> threadIDs;
    std::mutex mtx;

    // Create two threads and run threadFunction in each.
    std::thread thread1(mylib::insertStuff, 1, reference_wrapper(mtx), reference_wrapper(threadIDs));
    std::thread thread2(mylib::insertStuff, 2, reference_wrapper(mtx), reference_wrapper(threadIDs));
    std::thread thread3(mylib::insertStuff, 3, reference_wrapper(mtx), reference_wrapper(threadIDs));
    std::thread thread4(mylib::insertStuff, 4, reference_wrapper(mtx), reference_wrapper(threadIDs));

    // mylib::insertStuff(1, threadIDs);
    // mylib::insertStuff(2, threadIDs);

    // Wait for the threads to finish
    thread1.join();
    thread2.join();
    thread3.join();
    thread4.join();

    int num = 0;
    for (auto id : threadIDs) {
        std::cout << ++num << ": " << id << std::endl;
    }

    return 0;
}