#pragma once
#include <mutex>
#include <atomic>
#include <queue>
#include <condition_variable>

class JobQueue {
private:
    std::atomic_bool keep_running;
    std::atomic_bool has_work;
    std::condition_variable cv;
    std::mutex queue_lock;
    //todo: replace bool with whatever the data is going to be
    std::queue<bool> jqueue;
    // this class is the only thing that can instantiate itself now
    JobQueue(){}
public:
    static JobQueue& GetInstance();
    //todo: add data to signature
    bool Enqueue();
    void Process();
    void Stop();
};
