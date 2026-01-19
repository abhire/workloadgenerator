#include <workloadgenerator/bounded_blocking_queue.h>

#include <chrono>
#include <iostream>
#include <thread>

int main()
{
    using namespace std::chrono_literals;

    workloadgenerator::BoundedBlockingQueue<int> q(2);

    std::jthread producer([&] {
        for (int i = 1; i <=5 ; i++)
        {
            std::cout << "[producer] pushing " << i << std::endl;
            
            bool ok = q.push(i);// will block when full

            if (!ok)
            {
                std::cout << "[producer] queue closed, stopping." << std::endl;
                return;
            }         
        }

        std::cout << "[producer] done, closing" << std::endl;
        q.close();
    });

    std::jthread consumer([&] {
        while (true)
        {
            auto item = q.pop();
            if (!item)
            {
                std::cout << "[consumer] queue closed + empty, exit" << std::endl;
                return;
            }
            std::cout << " [consumer] got " << *item << std::endl;

            std::this_thread::sleep_for(300ms);// slow consumer for force backpressue;
        }
    });

}
