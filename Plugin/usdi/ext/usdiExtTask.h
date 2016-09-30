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

class VertexUpdateTaskManager
{
public:
    typedef VertexUpdateTask task_t;
    typedef std::vector<VertexUpdateTask> tasks_t;

    void queue(const VertexUpdateTask& t);
    void endQueing();
    void flush();
    bool isFlushing() const;
    void clear();

private:
    typedef tbb::spin_mutex::scoped_lock lock_t;

    tbb::spin_mutex m_mutex_queuing;
    tbb::spin_mutex m_mutex_flusing;
    tasks_t m_tasks_queing;
    tasks_t m_tasks_pending;
    tasks_t m_tasks_flushing;
    std::atomic_bool m_flushing = false;
};



class TaskManager
{
public:
    typedef void (*TaskFunc)(void *);

    TaskManager();
    ~TaskManager();
    handle_t createTask(TaskFunc task, void *arg);
    void destroyTask(handle_t h);
    void run(handle_t h);
    bool isRunning(handle_t h);
    void wait(handle_t h);

private:
    struct Task
    {
        TaskFunc func = nullptr;
        void *arg = nullptr;
        tbb::spin_mutex mutex;

        Task(TaskFunc f, void *a) : func(f), arg(a) {}
        usdiDefineCachedOperatorNew(Task, 128);
    };
    typedef std::unique_ptr<Task> TaskPtr;
    typedef tbb::spin_mutex::scoped_lock lock_t;

    Task* getTask(handle_t h);

    tbb::spin_mutex m_mutex;
    tbb::task_group m_group;
    HandleBasedVector<TaskPtr> m_tasks;
};

} // namespace usdi
