#include <job-queue.h>
#include <thread>

JobQueue& JobQueue::GetInstance() {
    static JobQueue jq;
    return jq;
}

void JobQueue::AddJob(std::function<void()> job) {
    queue_lock.lock();
    jqueue.emplace(job);
    queue_lock.unlock();
    cv.notify_all();
    has_work = true;
}

void JobQueue::Process() {
    keep_running = true;
    std::mutex wait_mtx;
    std::unique_lock<std::mutex> wait_lock(wait_mtx);
    // so long as this stays true we're gonna keep looping
    while(keep_running.load()){
        while(!has_work.load()){
            cv.wait(wait_lock);
        }
        queue_lock.lock();
        auto &job = jqueue.front();
        jqueue.pop();
        if(jqueue.empty()){
            has_work = false;
        }
        queue_lock.unlock();
        job();
    }
    // we can't leave unperformed jobs in the queue otherwise they will be forgotten, so we have to finish them before we can shut down fully
    while(!jqueue.empty()){
        auto &job = jqueue.front();
        job();
        jqueue.pop();
    }
}

void JobQueue::Stop() {
    keep_running = false;
}
/**/