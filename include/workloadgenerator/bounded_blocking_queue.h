#pragma once

#include <condition_variable>
#include <deque>
#include <mutex>



namespace workloadgenerator
{

    template <typename T>
    class BoundedBlockingQueue
    {
        public:
        explicit BoundedBlockingQueue(std::size_t capacity)
        : capacity_(capacity) {}


        //Blocks if queue is full, Returns false if queue is closed.

        bool push(T value)
        {
            std::unique_lock<std::mutex> lock(mutex_);

            cv_not_full_.wait(lock, [&] {
                return closed_ || queue_.size() < capacity_;
            });

            if (closed_)
                return false;

            queue_.push_back(std::move(value));
            cv_not_empty_.notify_one();
        }



        private:

        const std::size_t capacity_;
        mutable std::mutex mutex_;

        std::condition_variable cv_not_full_;
        std::condition_variable cv_not_empty_;

        std::deque<T> queue_;
        bool closed_ = false;
    };

} // namespace workloadgenerator