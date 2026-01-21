#include <functional>
#include <include/workloadgenerator/bounded_blocking_queue.h>
#include <cstddef>

namespace workloadgenerator
{


class ThreadPool
{

public:
    struct Config 
    {
        std::size_t threads = std::thread::hardware_concurrency() ? std::thread::hardware_concurrency() : 4;
        std::size_t queue_capacity = 1024;
    };







private:

using task = std::function<void()>;

void worker_loop();

Config cfg_;
BoundedBlockingQueue<task> queue_;
std::vector<std::thread> workers_;
std::atomic<bool> running_{true};

};

}