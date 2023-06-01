#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

namespace mylib
{
    // A function to be run in a separate thread
    void threadFunction(int threadID)
    {
        std::cout << "Howdy from thread " << threadID << "!\n";
    }

    void insertStuff(int threadID, std::mutex &mtx, std::vector<int> &threadIDs)
    {
        for (int i = 0; i < 10; i++)
        {
            {
                std::lock_guard<std::mutex> g(mtx);
                threadIDs.push_back(threadID);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

}