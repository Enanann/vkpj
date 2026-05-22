#pragma once

#include <thread>
#include <mutex>
#include <future>
#include <queue>
#include <condition_variable>
#include <functional>
#include <type_traits>
#include <memory>
#include <vector>
#include <iostream>

class ThreadPool {
public:
    ThreadPool(size_t sizes = std::thread::hardware_concurrency() - 1);
    ~ThreadPool();

    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::invoke_result_t<F, Args...>>
    {
        using return_type = typename std::invoke_result_t<F, Args...>;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            );

        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(mMutex);

            // don't allow enqueueing after stopping the pool
            if(mStop)
                throw std::runtime_error("enqueue on stopped ThreadPool");

            mQueue.emplace([task]() { 
                (*task)(); 
            });
        }
        mCV.notify_one();
        return res;
    }

private:
    std::vector<std::thread> mWorkers;
    std::mutex mMutex;
    std::condition_variable mCV;
    std::queue<std::function<void()>> mQueue;
    bool mStop;
};
