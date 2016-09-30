#include "pch.h"
#include "usdiInternal.h"
#include "usdiExtTask.h"
#include "usdiUtils.h"

namespace usdi {

VertexUpdateTask::VertexUpdateTask(const usdi::MeshData *mesh_data, MapContext *ctx_vb, MapContext *ctx_ib)
    : m_mesh_data(mesh_data), m_ctx_vb(ctx_vb), m_ctx_ib(ctx_ib)
{
    m_ctx_vb->mode = gi::MapMode::Write;
    m_ctx_vb->type = gi::BufferType::Vertex;
    m_ctx_vb->keep_staging_resource = true;

    m_ctx_ib->mode = gi::MapMode::Write;
    m_ctx_ib->type = gi::BufferType::Vertex;
    m_ctx_ib->keep_staging_resource = true;
}

VertexUpdateTask::~VertexUpdateTask()
{
    auto ifs = gi::GetGraphicsInterface();
    ifs->releaseStagingResource(*m_ctx_vb);
    ifs->releaseStagingResource(*m_ctx_ib);
}

void VertexUpdateTask::map()
{
    auto ifs = gi::GetGraphicsInterface();
    ifs->mapBuffer(*m_ctx_vb);
    if (m_mesh_data->indices_triangulated) {
        ifs->mapBuffer(*m_ctx_ib);
    }
}

void VertexUpdateTask::copy()
{
    auto *ifs = gi::GetGraphicsInterface();
    auto& buf = GetTemporaryBuffer();

    if (m_ctx_vb->data_ptr) {
        if (m_mesh_data->uvs) {
            using vertex_t = vertex_v3n3u2;
            vertex_t::source_t src = { m_mesh_data->points, m_mesh_data->normals, m_mesh_data->uvs };
            InterleaveBuffered(buf, src, (size_t)m_mesh_data->num_points);
        }
        else {
            using vertex_t = vertex_v3n3;
            vertex_t::source_t src = { m_mesh_data->points, m_mesh_data->normals };
            InterleaveBuffered(buf, src, (size_t)m_mesh_data->num_points);
        }
        memcpy(m_ctx_vb->data_ptr, buf.cdata(), buf.size());
    }

    if (m_ctx_ib->data_ptr && m_mesh_data->indices_triangulated) {
        // need to convert 32 bit IB -> 16 bit IB...
        using index_t = uint16_t;
        buf.resize(sizeof(index_t) * m_mesh_data->num_indices_triangulated);
        index_t *indices = (index_t*)buf.cdata();
        for (size_t i = 0; i < m_mesh_data->num_indices_triangulated; ++i) {
            indices[i] = (index_t)m_mesh_data->indices_triangulated[i];
        }
        memcpy(m_ctx_ib->data_ptr, buf.cdata(), buf.size());
    }
}

void VertexUpdateTask::unmap()
{
    auto ifs = gi::GetGraphicsInterface();
    ifs->unmapBuffer(*m_ctx_vb);
    ifs->unmapBuffer(*m_ctx_ib);
}



void VertexUpdateTaskManager::queue(const VertexUpdateTask& t)
{
    lock_t l(m_mutex_queuing);
    m_tasks_queing.push_back(t);
}

void VertexUpdateTaskManager::endQueing()
{
    lock_t lq(m_mutex_queuing);
    m_tasks_pending.swap(m_tasks_queing);
    m_tasks_queing.clear();
}

void VertexUpdateTaskManager::flush()
{
    {
        lock_t lq(m_mutex_queuing);
        m_tasks_flushing.swap(m_tasks_pending);
    }

    lock_t lf(m_mutex_flusing);
    if (m_tasks_flushing.empty()) { return; }

    m_flushing = true;
    for (auto& t : m_tasks_flushing) { t.map(); }
#ifdef usdiDbgForceSingleThread
    for (auto& t : m_tasks_flushing) { t.copy(); }
#else
    tbb::parallel_for_each(m_tasks_flushing, [](auto& t) { t.copy(); });
#endif
    for (auto& t : m_tasks_flushing) { t.unmap(); }
    m_tasks_flushing.clear();
    m_flushing = false;
}

bool VertexUpdateTaskManager::isFlushing() const
{
    return m_flushing;
}

void VertexUpdateTaskManager::clear()
{
    lock_t lq(m_mutex_queuing);
    lock_t lf(m_mutex_flusing);

    m_tasks_queing.clear();
    m_tasks_pending.clear();
    m_tasks_flushing.clear();
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

handle_t TaskManager::createTask(TaskFunc func, void *arg)
{
    auto *ptr = new Task(func, arg);

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
        task->func(task->arg);
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
