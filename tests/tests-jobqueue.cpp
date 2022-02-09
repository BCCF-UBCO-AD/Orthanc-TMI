#include <gtest/gtest.h>
#include <thread>
#include <job-queue.h>
#include "common.h"

TEST(jobqueue, complete_job) {
    std::thread job_thread = std::thread(&JobQueue::Process, &JobQueue::GetInstance());
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::atomic<bool> job_completed = false;
    ASSERT_TRUE(JobQueue::GetInstance().AddJob([&](){
        job_completed = true;
    }));
    JobQueue::GetInstance().Stop();
    job_thread.join();
    ASSERT_TRUE(job_completed.load());
}