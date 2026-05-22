#include "threadpool.hpp"

ThreadPool::ThreadPool(size_t sizes) : mStop{false} {
    for (size_t i{0}; i < sizes; ++i) {
        mWorkers.emplace_back([this] {
            for (;;) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->mMutex);
                    this->mCV.wait(lock, [this] {return this->mStop || !this->mQueue.empty();});
                    if (this->mStop && this->mQueue.empty()) {
                        return;
                    }
                    task = std::move(this->mQueue.front());
                    this->mQueue.pop();
                }

                task();
            }
        });
    }
}

// the destructor joins all threads
ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(mMutex);
        mStop = true;
    }
    mCV.notify_all();
    for(std::thread& worker: mWorkers)
        worker.join();
}
