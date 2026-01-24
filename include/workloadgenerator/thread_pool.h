#include <functional>
#include <workloadgenerator/bounded_blocking_queue.h>
#include <cstddef>
#include <future>
#include <atomic>
#include <tuple>
#include <type_traits>
#include <utility>
#include <stdexcept>

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

        explicit ThreadPool(Config cfg);
        ~ThreadPool();

        ThreadPool() = delete;

        ThreadPool &operator=(const ThreadPool &) = delete;

        template <typename F, typename... Args>
        auto submit(F &&f, Args &&...args) -> std::future<std::invoke_result_t<F, Args...>>;

    private:
        using task = std::function<void()>;

        void worker_loop();

        Config cfg_;
        BoundedBlockingQueue<task> queue_;
        std::vector<std::thread> workers_;
        std::atomic<bool> running_{true};
    };

    template <typename F, typename... Args>
    auto ThreadPool::submit(F &&f, Args &&...args)
        -> std::future<std::invoke_result_t<F, Args...>>
    {

        using R = std::invoke_result_t<F, Args...>;

        if (!running_.load(std::memory_order_acquire))
        {
            std::promise<R> p;
            p.set_exception(std::make_exception_ptr(std::runtime_error("ThreadPool is stopped")));
            return p.get_future();
        }

        auto task_ptr = std::make_shared<std::packaged_task<R()>>(
            [fn = std::forward<F>(f),
             tup = std::make_tuple(std::forward<Args>(args)...)]() mutable -> R
            {
                return std::apply(std::move(fn), std::move(tup));
            });

        task wrapper = [task_ptr]()
        { (*task_ptr)(); };

        // push() blocks if queue is full; returns false if closed
        if (!queue_.push(std::move(wrapper)))
        {
            std::promise<R> p;
            p.set_exception(std::make_exception_ptr(std::runtime_error("ThreadPool queue closed")));
            return p.get_future();
        }

        return task_ptr->get_future();
    }

} // namespace workloadgenerator
