#pragma once

#include <mutex>
#include <atomic>
#include <queue>
#include <condition_variable>
#include <functional>

class JobQueue {
private:
    std::atomic_bool keep_running;
    std::atomic_bool has_work;
    std::condition_variable cv;
    std::mutex queue_lock;
    std::queue<std::function<void()>> jqueue;
    // this class is the only thing that can instantiate itself now
    JobQueue(){}
public:
    static JobQueue& GetInstance();
    void AddJob(std::function<void()> job);
    void Process();
    void Stop();
};
/**/