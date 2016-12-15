#pragma once

#ifdef usdiEnableUnityExtension
#ifdef usdiEnableGraphicsInterface
    #include "GraphicsInterface/GraphicsInterface.h"
#endif // usdiEnableGraphicsInterface
#include "etc/HandleBasedVector.h"
#include "etc/FixedAllocator.h"
#include "usdiExt.h"

namespace usdi {

class MonoThreadScope
{
public:
    MonoThreadScope();
    ~MonoThreadScope();
};


#ifdef usdiEnableGraphicsInterface
using MapContext = gi::MapContext;

class VertexUpdateCommand
{
public:
    VertexUpdateCommand(const char *dbg_name);
    ~VertexUpdateCommand();

    void update(const usdi::MeshData *data, void *vb, void *ib);
    void update(const usdi::SubmeshData *data, void *vb, void *ib);
    bool isDirty() const;

    void map();
    void copy();
    void unmap();
    void clearDirty();

    usdiDefineCachedOperatorNew(VertexUpdateCommand, 256);

private:
    typedef tbb::spin_mutex::scoped_lock lock_t;

    std::string m_dbg_name;

    const float3 *m_src_points = nullptr;
    const float3 *m_src_normals = nullptr;
    const float2 *m_src_uvs = nullptr;
    const float4 *m_src_tangents = nullptr;
    const int    *m_src_indices = nullptr;
    int          m_num_points = 0;
    int          m_num_indices = 0;

    MapContext m_ctx_vb;
    MapContext m_ctx_ib;
    std::atomic_bool m_dirty;
};

class VertexCommandManager
{
public:
    typedef VertexUpdateCommand Command;
    typedef std::unique_ptr<VertexUpdateCommand> CommandPtr;

    static VertexCommandManager& getInstance();

    Handle createCommand(const char *dbg_name = "");
    void destroyCommand(Handle h);
    void update(Handle h, const usdi::MeshData *src, void *vb, void *ib);
    void update(Handle h, const usdi::SubmeshData *src, void *vb, void *ib);

    void process();
    void wait();

private:
    typedef tbb::spin_mutex::scoped_lock lock_t;

    VertexUpdateCommand* get(Handle h);

    tbb::spin_mutex                 m_mutex_processing;
    HandleBasedVector<CommandPtr>   m_commands;
    std::vector<Command*>           m_dirty_commands;
};
#endif // usdiEnableGraphicsInterface


class Task
{
public:
    Task(const std::function<void()>& f, const char *n = "");
    void run(bool async = true);
    bool isRunning();
    void wait();

    usdiDefineCachedOperatorNew(Task, 256);

private:
    static tbb::task_group s_task_group;

    std::string m_dbg_name;
    std::function<void()> m_func;
    tbb::spin_mutex m_mutex;
};

} // namespace usdi
#endif // usdiEnableUnityExtension
