#pragma once

namespace cave {

class TaskContext;

class IAsyncTask {
public:
    virtual ~IAsyncTask() = default;

    // Name shown in UI and snapshots.
    virtual const char* Name() const = 0;

    // Runs on a dedicated async worker thread.
    // Periodically check ctx.IsCancelRequested().
    virtual void Run(TaskContext& p_ctx) = 0;
};

}  // namespace cave
