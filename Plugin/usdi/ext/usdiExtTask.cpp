#include "pch.h"
#include "usdiInternal.h"
#include "usdiExtTask.h"
#include "usdiUtils.h"
#include "etc/Mono.h"

namespace usdi {

VertexUpdateCommand::VertexUpdateCommand(const char *dbg_name)
    : m_dbg_name(dbg_name)
{
    m_ctx_vb.mode = gi::MapMode::Write;
    m_ctx_vb.type = gi::BufferType::Vertex;
    m_ctx_vb.keep_staging_resource = true;

    m_ctx_ib.mode = gi::MapMode::Write;
    m_ctx_ib.type = gi::BufferType::Vertex;
    m_ctx_ib.keep_staging_resource = true;
}

VertexUpdateCommand::~VertexUpdateCommand()
{
    auto ifs = gi::GetGraphicsInterface();
    ifs->releaseStagingResource(m_ctx_vb);
    ifs->releaseStagingResource(m_ctx_ib);
}

void VertexUpdateCommand::update(const usdi::MeshData *mesh_data, void *vb, void *ib)
{
    m_mesh_data = *mesh_data;
    m_ctx_vb.resource = vb;
    m_ctx_ib.resource = ib;

    m_dirty = true;
}

bool VertexUpdateCommand::isDirty() const
{
    return m_dirty;
}

void VertexUpdateCommand::map()
{
    auto ifs = gi::GetGraphicsInterface();
    ifs->mapBuffer(m_ctx_vb);
    if (m_mesh_data.indices_triangulated) {
        ifs->mapBuffer(m_ctx_ib);
    }
}

void VertexUpdateCommand::copy()
{
    auto *ifs = gi::GetGraphicsInterface();
    auto& buf = GetTemporaryBuffer();
    auto& mesh_data = m_mesh_data;
    auto& ctx_vb = m_ctx_vb;
    auto& ctx_ib = m_ctx_ib;

    if (ctx_vb.data_ptr) {
        if (m_mesh_data.uvs) {
            using vertex_t = vertex_v3n3u2;
            vertex_t::source_t src = { m_mesh_data.points, m_mesh_data.normals, m_mesh_data.uvs };
            InterleaveBuffered(buf, src, (size_t)mesh_data.num_points);
        }
        else {
            using vertex_t = vertex_v3n3;
            vertex_t::source_t src = { mesh_data.points, mesh_data.normals };
            InterleaveBuffered(buf, src, (size_t)mesh_data.num_points);
        }
        memcpy(ctx_vb.data_ptr, buf.cdata(), buf.size());
    }

    if (ctx_ib.data_ptr && mesh_data.indices_triangulated) {
        // need to convert 32 bit IB -> 16 bit IB...
        using index_t = uint16_t;
        buf.resize(sizeof(index_t) * mesh_data.num_indices_triangulated);
        index_t *indices = (index_t*)buf.cdata();
        for (size_t i = 0; i < mesh_data.num_indices_triangulated; ++i) {
            indices[i] = (index_t)mesh_data.indices_triangulated[i];
        }
        memcpy(ctx_ib.data_ptr, buf.cdata(), buf.size());
    }
}

void VertexUpdateCommand::unmap()
{
    auto ifs = gi::GetGraphicsInterface();
    ifs->unmapBuffer(m_ctx_vb);
    ifs->unmapBuffer(m_ctx_ib);
}

void VertexUpdateCommand::clearDirty()
{
    m_dirty = false;
}


VertexUpdateCommand* VertexCommandManager::get(handle_t h)
{
    return m_commands.get(h).get();
}

handle_t VertexCommandManager::createCommand(const char *dbg_name)
{
    lock_t l(m_mutex_processing);
    return m_commands.push(CommandPtr(new Command(dbg_name)));
}

void VertexCommandManager::destroyCommand(handle_t h)
{
    lock_t l(m_mutex_processing);
    m_commands.pull(h);
}

void VertexCommandManager::update(handle_t h, const usdi::MeshData *src, void *vb, void *ib)
{
    if (auto *cmd = get(h)) {
        cmd->update(src, vb, ib);
    }
}

void VertexCommandManager::process()
{
    lock_t l(m_mutex_processing);

    auto& dirty = m_dirty_commands;
    for (auto& t : m_commands.getValues()) {
        if (t && t->isDirty()) {
            dirty.push_back(t.get());
        }
    }

    if (!dirty.empty()) {
        for (auto& t : dirty) { t->map(); }

#ifdef usdiDbgForceSingleThread
        for (auto& t : dirty) { t->copy(); }
#else
        size_t grain = std::max<size_t>(dirty.size()/32, 1);
        tbb::parallel_for(tbb::blocked_range<size_t>(0, dirty.size(), grain), [&dirty](const auto& r) {
            for (size_t i = r.begin(); i != r.end(); ++i) {
                dirty[i]->copy();
            }
        });
#endif

        for (auto& t : dirty) { t->unmap(); t->clearDirty(); }
        dirty.clear();
    }
}

void VertexCommandManager::wait()
{
    lock_t l(m_mutex_processing);
}



TaskManager::TaskManager()
{
}

TaskManager::~TaskManager()
{
}

TaskManager::Task* TaskManager::getTask(handle_t h)
{
    if (h == 0) { return nullptr; }

    lock_t l(m_mutex);
    return m_tasks.get(h).get();
}

handle_t TaskManager::createTask(TaskFunc func, void *arg, const char *name)
{
    auto *ptr = new Task(func, arg, name);

    handle_t ret = 0;
    {
        lock_t l(m_mutex);
        ret = m_tasks.push(TaskPtr(ptr));
    }
    return ret;
}

void TaskManager::destroyTask(handle_t h)
{
    lock_t l(m_mutex);
    m_tasks.pull(h);
}

void TaskManager::run(handle_t h)
{
    Task *task = getTask(h);
    if (!task) { return; }

    task->mutex.lock();
    m_group.run([task]() {
        {
            //MonoScope mscope;
            task->func(task->arg);
        }
        task->mutex.unlock();
    });
}

bool TaskManager::isRunning(handle_t h)
{
    Task *task = getTask(h);
    if (!task) { return false; }

    if (task->mutex.try_lock()) {
        task->mutex.unlock();
        return false;
    }
    else {
        return true;
    }
}

void TaskManager::wait(handle_t h)
{
    Task *task = getTask(h);
    if (!task) { return; }

    if (task) {
        task->mutex.lock();
        task->mutex.unlock();
    }
}

} // namespace usdi
