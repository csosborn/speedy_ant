#include <mutex>
#include <vector>

namespace mylib
{

    // A function to be run in a separate thread
    void threadFunction(int threadID);

    void insertStuff(int threadID, std::mutex& mtx, std::vector<int>& threadIDs);

}