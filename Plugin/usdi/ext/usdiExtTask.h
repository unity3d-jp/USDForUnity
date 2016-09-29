#pragma once

#include "GraphicsInterface/GraphicsInterface.h"
#include "etc/HandleBasedVector.h"
#include "etc/FixedAllocator.h"

namespace usdi {

struct MapContext : gi::MapContext {};

class VertexUpdateTask
{

public:
    VertexUpdateTask(const usdi::MeshData *mesh_data, MapContext *ctx_vb, MapContext *ctx_ib);
    ~VertexUpdateTask();

    void map();
    void copy();
    void unmap();

private:
    const usdi::MeshData *m_mesh_data;
    MapContext *m_ctx_vb;
    MapContext *m_ctx_ib;
};

class VertexUpdateTaskQueue
{
public:
    typedef VertexUpdateTask task_t;
    typedef std::vector<VertexUpdateTask> tasks_t;

    void push(const VertexUpdateTask& t);
    void flush();
    bool isFlushing() const;
    void clear();

private:
    typedef tbb::spin_mutex::scoped_lock lock_t;

    tbb::spin_mutex m_mutex;
    tasks_t m_tasks;
    std::atomic_bool m_flushing = false;
};



class TaskGroup
{
public:
    typedef void (*TaskFunc)(void *);

    TaskGroup();
    ~TaskGroup();
    handle_t run(TaskFunc task, void *arg);
    bool isRunning(handle_t h);
    void wait(handle_t h);
    void waitAll();

private:
    struct Task
    {
        TaskFunc func = nullptr;
        tbb::spin_mutex mutex;

        Task(TaskFunc f) : func(f) {}
        usdiDefineCachedOperatorNew(Task, 128);
    };
    typedef std::unique_ptr<Task> TaskPtr;
    typedef tbb::spin_mutex::scoped_lock lock_t;

    tbb::spin_mutex m_mutex;
    tbb::task_group m_group;
    HandleBasedVector<TaskPtr> m_tasks;
};

} // namespace usdi
