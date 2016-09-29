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


void VertexUpdateTaskQueue::push(const VertexUpdateTask& t)
{
    lock_t l(m_mutex);
    m_tasks.push_back(t);
}

void VertexUpdateTaskQueue::flush()
{
    lock_t l(m_mutex);


    m_flushing = true;
    for (auto& t : m_tasks) { t.map(); }
#ifdef usdiDbgForceSingleThread
    for (auto& t : m_tasks) { t.copy(); }
#else
    tbb::parallel_for_each(m_tasks, [](auto& t) { t.copy(); });
#endif
    for (auto& t : m_tasks) { t.unmap(); }
    m_tasks.clear();
    m_flushing = false;
}

bool VertexUpdateTaskQueue::isFlushing() const
{
    return m_flushing;
}

void VertexUpdateTaskQueue::clear()
{
    lock_t l(m_mutex);
    m_tasks.clear();
}



TaskGroup::TaskGroup()
{
}

TaskGroup::~TaskGroup()
{
    waitAll();
}

handle_t TaskGroup::run(TaskFunc tf, void *arg)
{
    auto *ptr = new Task(tf);
    ptr->mutex.lock();

    handle_t ret = 0;
    {
        lock_t l(m_mutex);
        ret = m_tasks.push(TaskPtr(ptr));
    }
    m_group.run([ptr, arg]() {
        ptr->func(arg);
        ptr->mutex.unlock();
    });

    return ret;
}

bool TaskGroup::isRunning(handle_t h)
{
    if (h == 0) { return false; }

    Task *ptr = nullptr;
    {
        lock_t l(m_mutex);
        ptr = m_tasks.get(h).get();
    }
    if (ptr && ptr->mutex.try_lock()) {
        ptr->mutex.unlock();
        return true;
    }
    return false;
}

void TaskGroup::wait(handle_t h)
{
    if (h == 0) { return; }

    TaskPtr t;
    {
        lock_t l(m_mutex);
        t = m_tasks.pull(h);
    }
    if (t) {
        t->mutex.lock();
    }
}

void TaskGroup::waitAll()
{
    lock_t l(m_mutex);
    m_group.wait();
    // todo: clear container
}

} // namespace usdi
