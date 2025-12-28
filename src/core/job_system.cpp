#include "lunaris/core/job_system.h"

namespace lunaris {

JobSystem::JobSystem()
    : _pending_count(0)
    , _worker_count(0)
    , _running(false)
    , _next_id(1) {
    for (uint32_t i = 0; i < MAX_PENDING_JOBS; ++i) {
        _pending_jobs[i].job = nullptr;
        _pending_jobs[i].owns_memory = false;
    }
    for (uint32_t i = 0; i < MAX_WORKERS; ++i) {
        _workers[i] = nullptr;
    }
}

JobSystem::~JobSystem() {
    shutdown();
}

void JobSystem::init(uint32_t worker_count) {
    if (_running.load()) {
        return;
    }

    if (worker_count == 0) {
        worker_count = std::thread::hardware_concurrency();
        if (worker_count > 1) {
            worker_count -= 1;
        }
    }

    if (worker_count > MAX_WORKERS) {
        worker_count = MAX_WORKERS;
    }

    _running.store(true);
    _worker_count = worker_count;

    for (uint32_t i = 0; i < _worker_count; ++i) {
        _workers[i] = new std::thread(&JobSystem::worker_thread, this, i);
    }
}

void JobSystem::shutdown() {
    if (!_running.load()) {
        return;
    }

    _running.store(false);
    _queue_cv.notify_all();

    for (uint32_t i = 0; i < _worker_count; ++i) {
        if (_workers[i]) {
            _workers[i]->join();
            delete _workers[i];
            _workers[i] = nullptr;
        }
    }

    std::lock_guard<std::mutex> lock(_queue_mutex);
    for (uint32_t i = 0; i < MAX_PENDING_JOBS; ++i) {
        if (_pending_jobs[i].job && _pending_jobs[i].owns_memory) {
            delete _pending_jobs[i].job;
        }
        _pending_jobs[i].job = nullptr;
        _pending_jobs[i].owns_memory = false;
    }
    _pending_count = 0;
}

JobID JobSystem::generate_id() {
    return _next_id.fetch_add(1);
}

JobID JobSystem::submit(Job* job) {
    if (!job || !_running.load()) {
        return INVALID_JOB_ID;
    }

    std::lock_guard<std::mutex> lock(_queue_mutex);

    if (_pending_count >= MAX_PENDING_JOBS) {
        return INVALID_JOB_ID;
    }

    JobID id = generate_id();
    job->set_id(id);
    job->set_state(JobState::Pending);

    uint32_t insert_idx = 0;
    for (uint32_t i = 0; i < MAX_PENDING_JOBS; ++i) {
        if (_pending_jobs[i].job == nullptr) {
            insert_idx = i;
            break;
        }
    }

    _pending_jobs[insert_idx].job = job;
    _pending_jobs[insert_idx].owns_memory = true;
    _pending_count++;

    _queue_cv.notify_one();

    return id;
}

Job* JobSystem::pop_job() {
    std::unique_lock<std::mutex> lock(_queue_mutex);

    _queue_cv.wait(lock, [this]() {
        return !_running.load() || _pending_count > 0;
    });

    if (!_running.load() && _pending_count == 0) {
        return nullptr;
    }

    Job* best_job = nullptr;
    uint32_t best_idx = 0;
    JobPriority best_priority = JobPriority::Low;

    for (uint32_t i = 0; i < MAX_PENDING_JOBS; ++i) {
        if (_pending_jobs[i].job && _pending_jobs[i].job->get_state() == JobState::Pending) {
            JobPriority p = _pending_jobs[i].job->get_priority();
            if (!best_job || static_cast<uint8_t>(p) > static_cast<uint8_t>(best_priority)) {
                best_job = _pending_jobs[i].job;
                best_idx = i;
                best_priority = p;
            }
        }
    }

    if (best_job) {
        best_job->set_state(JobState::Running);
        _pending_jobs[best_idx].job = nullptr;
        _pending_jobs[best_idx].owns_memory = false;
        _pending_count--;
    }

    return best_job;
}

void JobSystem::worker_thread(uint32_t worker_id) {
    (void)worker_id;

    while (_running.load()) {
        Job* job = pop_job();
        if (!job) {
            continue;
        }

        job->execute();
        job->set_state(JobState::Completed);

        delete job;
    }
}

void JobSystem::wait(JobID id) {
    while (!is_complete(id) && _running.load()) {
        std::this_thread::yield();
    }
}

void JobSystem::wait_all() {
    while (get_pending_count() > 0 && _running.load()) {
        std::this_thread::yield();
    }
}

bool JobSystem::is_complete(JobID id) const {
    std::lock_guard<std::mutex> lock(_queue_mutex);

    for (uint32_t i = 0; i < MAX_PENDING_JOBS; ++i) {
        if (_pending_jobs[i].job && _pending_jobs[i].job->get_id() == id) {
            return _pending_jobs[i].job->is_complete();
        }
    }

    return true;
}

uint32_t JobSystem::get_pending_count() const {
    std::lock_guard<std::mutex> lock(_queue_mutex);
    return _pending_count;
}

Job* JobSystem::find_job(JobID id) const {
    for (uint32_t i = 0; i < MAX_PENDING_JOBS; ++i) {
        if (_pending_jobs[i].job && _pending_jobs[i].job->get_id() == id) {
            return _pending_jobs[i].job;
        }
    }
    return nullptr;
}

}
