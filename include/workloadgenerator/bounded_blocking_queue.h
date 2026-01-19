#pragma once

#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>
#include <cstddef>


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

            return true;
        }

        std::optional<T> pop()
        {
            std::unique_lock<std::mutex> lock(mutex_);

            //Wait until there is an item or the queue is closed.

            cv_not_empty_.wait(lock, [&] {
                return closed_ || !queue_.empty();
            });

            if (queue_.empty())
                return std::nullopt;

            auto res = std::move(queue_.front());
            queue_.pop_front();

            // Wake one waiting producer (space is available now)
            cv_not_full_.notify_one();

            return res;
        }

        void close()
        {
            std::lock_guard<std::mutex> lock(mutex_);

            closed_ = true;
            cv_not_empty_.notify_all();
            cv_not_full_.notify_all();
        }

        std::size_t size() const 
        {
            std::lock_guard<std::mutex> lock(mutex_);
            return queue_.size();
        }

        bool closed() const 
        {
            std::lock_guard<std::mutex> lock(mutex_);
            return closed_;
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