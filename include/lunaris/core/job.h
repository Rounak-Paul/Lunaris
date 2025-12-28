#pragma once

#include <cstdint>
#include <atomic>

namespace lunaris {

using JobID = uint32_t;
constexpr JobID INVALID_JOB_ID = 0;

enum class JobPriority : uint8_t {
    Low = 0,
    Normal = 1,
    High = 2,
    Critical = 3
};

enum class JobState : uint8_t {
    Pending,
    Running,
    Completed,
    Failed,
    Cancelled
};

class Job {
public:
    Job() 
        : _id(INVALID_JOB_ID)
        , _priority(JobPriority::Normal)
        , _state(JobState::Pending) {}
    
    virtual ~Job() = default;

    virtual void execute() = 0;
    virtual const char* get_name() const { return "Job"; }

    JobID get_id() const { return _id; }
    JobPriority get_priority() const { return _priority; }
    JobState get_state() const { return _state.load(); }

    void set_priority(JobPriority priority) { _priority = priority; }

    bool is_complete() const { 
        JobState s = _state.load();
        return s == JobState::Completed || s == JobState::Failed || s == JobState::Cancelled; 
    }

protected:
    void set_state(JobState state) { _state.store(state); }

private:
    friend class JobSystem;

    void set_id(JobID id) { _id = id; }

    JobID _id;
    JobPriority _priority;
    std::atomic<JobState> _state;
};

template<typename Func>
class LambdaJob : public Job {
public:
    LambdaJob(Func&& func, const char* name = "LambdaJob")
        : _func(static_cast<Func&&>(func))
        , _name(name) {}

    void execute() override { _func(); }
    const char* get_name() const override { return _name; }

private:
    Func _func;
    const char* _name;
};

}
