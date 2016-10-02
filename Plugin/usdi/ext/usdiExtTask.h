#pragma once

#include "GraphicsInterface/GraphicsInterface.h"
#include "etc/HandleBasedVector.h"
#include "etc/FixedAllocator.h"

namespace usdi {

using MapContext = gi::MapContext;

class VertexUpdateCommand
{

public:
    VertexUpdateCommand(const char *dbg_name);
    ~VertexUpdateCommand();

    void update(const usdi::MeshData *mesh_data, void *vb, void *ib);
    bool isDirty() const;

    void map();
    void copy();
    void unmap();
    void clearDirty();

    usdiDefineCachedOperatorNew(VertexUpdateCommand, 256);

private:
    typedef tbb::spin_mutex::scoped_lock lock_t;

    std::string m_dbg_name;
    usdi::MeshData m_mesh_data;
    MapContext m_ctx_vb;
    MapContext m_ctx_ib;
    std::atomic_bool m_dirty;
};

class VertexCommandManager
{
public:
    typedef VertexUpdateCommand Command;
    typedef std::unique_ptr<VertexUpdateCommand> CommandPtr;

    handle_t createCommand(const char *dbg_name);
    void destroyCommand(handle_t h);
    void update(handle_t h, const usdi::MeshData *src, void *vb, void *ib);

    void process();
    void wait();

private:
    typedef tbb::spin_mutex::scoped_lock lock_t;

    VertexUpdateCommand* get(handle_t h);

    tbb::spin_mutex                 m_mutex_processing;
    HandleBasedVector<CommandPtr>   m_commands;
    std::vector<Command*>           m_dirty_commands;
};



class TaskManager
{
public:
    typedef std::function<void()> TaskFunc;

    TaskManager();
    ~TaskManager();
    handle_t createTask(TaskFunc task, const char *name = "");
    void destroyTask(handle_t h);
    void run(handle_t h);
    bool isRunning(handle_t h);
    void wait(handle_t h);

private:
    struct Task
    {
        std::string dbg_name;
        std::function<void()> func;
        tbb::spin_mutex mutex;

        Task(const TaskFunc& f, const char *n) : func(f), dbg_name(n) {}
        usdiDefineCachedOperatorNew(Task, 256);
    };
    typedef std::unique_ptr<Task> TaskPtr;
    typedef tbb::spin_mutex::scoped_lock lock_t;

    Task* getTask(handle_t h);

    tbb::spin_mutex m_mutex;
    tbb::task_group m_group;
    HandleBasedVector<TaskPtr> m_tasks;
};

} // namespace usdi
