#pragma once
#include <mutex>
#include <atomic>
#include <queue>

class JobQueue {
private:
    //todo: replace bool with whatever the data is going to be
    std::queue<bool> jqueue;
    std::mutex queue_lock;
    std::atomic_bool keep_running;
    // this class is the only thing that can instantiate itself now
    JobQueue(){}
public:
    static JobQueue& GetInstance();
    //todo: add data to signature
    bool Enqueue();
    void Process();
    void Stop();
};
