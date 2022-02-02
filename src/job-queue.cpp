#include <job-queue.h>
#include <thread>

JobQueue& JobQueue::GetInstance() {
    static JobQueue jq;
    return jq;
}

bool JobQueue::Enqueue() {
    // check the atomic bool, if it indicates the job queue is still running we can add to the queue
    if(keep_running.load()){
        queue_lock.lock();
        jqueue.emplace(true);
        queue_lock.unlock();
        cv.notify_all();
        has_work = true;
        return true;
    }
    // we return false to indicate the queue didn't accept the job
    return false;
}

void JobQueue::Process() {
    keep_running = true;
    std::mutex wait_mtx;
    std::unique_lock<std::mutex> wait_lock(wait_mtx);
    // so long as this stays true we're gonna keep looping
    while(keep_running.load()){
        if(!has_work.load()){
            cv.wait(wait_lock);
        }
        queue_lock.lock();
        auto &job = jqueue.front();
        jqueue.pop();
        queue_lock.unlock();
        // todo: do stuff with the job
    }
    // todo: we can't leave unperformed jobs in the queue otherwise they will be forgotten, so we have to finish them before we can shut down fully
    while(!jqueue.empty()){
        auto &job = jqueue.front();
        // todo: do stuff with the job
        jqueue.pop();
    }
}

void JobQueue::Stop() {
    keep_running = false;
}