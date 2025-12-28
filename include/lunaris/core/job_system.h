#pragma once

#include "lunaris/core/job.h"
#include <mutex>
#include <condition_variable>
#include <thread>

namespace lunaris {

class JobSystem {
public:
    static constexpr uint32_t MAX_PENDING_JOBS = 1024;
    static constexpr uint32_t MAX_WORKERS = 16;

    JobSystem();
    ~JobSystem();

    void init(uint32_t worker_count = 0);
    void shutdown();

    JobID submit(Job* job);

    template<typename Func>
    JobID submit_lambda(Func&& func, const char* name = "LambdaJob", JobPriority priority = JobPriority::Normal) {
        auto* job = new LambdaJob<Func>(static_cast<Func&&>(func), name);
        job->set_priority(priority);
        return submit(job);
    }

    void wait(JobID id);
    void wait_all();

    bool is_complete(JobID id) const;
    uint32_t get_pending_count() const;
    uint32_t get_worker_count() const { return _worker_count; }

private:
    void worker_thread(uint32_t worker_id);
    Job* pop_job();
    Job* find_job(JobID id) const;
    JobID generate_id();

    struct JobEntry {
        Job* job;
        bool owns_memory;
    };

    JobEntry _pending_jobs[MAX_PENDING_JOBS];
    uint32_t _pending_count;
    mutable std::mutex _queue_mutex;
    std::condition_variable _queue_cv;

    std::thread* _workers[MAX_WORKERS];
    uint32_t _worker_count;

    std::atomic<bool> _running;
    std::atomic<JobID> _next_id;
};

}
