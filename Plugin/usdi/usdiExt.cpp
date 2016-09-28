#include "pch.h"
#include "usdiInternal.h"
#include "usdiUtils.h"
#include "HandleBasedVector.h"
#include "GraphicsInterface/GraphicsInterface.h"


namespace usdi {





struct MapContext : gi::MapContext {};

struct VertexUpdateTask
{
    const usdi::MeshData *m_mesh_data;
    gi::MapContext *m_ctx_vb;
    gi::MapContext *m_ctx_ib;

    VertexUpdateTask(const usdi::MeshData *mesh_data, gi::MapContext *ctx_vb, gi::MapContext *ctx_ib)
        : m_mesh_data(mesh_data), m_ctx_vb(ctx_vb), m_ctx_ib(ctx_ib)
    {
        m_ctx_vb->mode = gi::MapMode::Write;
        m_ctx_vb->type = gi::BufferType::Vertex;
        m_ctx_vb->keep_staging_resource = true;

        m_ctx_ib->mode = gi::MapMode::Write;
        m_ctx_ib->type = gi::BufferType::Vertex;
        m_ctx_ib->keep_staging_resource = true;
    }

    ~VertexUpdateTask()
    {
        auto ifs = gi::GetGraphicsInterface();
        ifs->releaseStagingResource(*m_ctx_vb);
        ifs->releaseStagingResource(*m_ctx_ib);
    }

    void map()
    {
        auto ifs = gi::GetGraphicsInterface();
        ifs->mapBuffer(*m_ctx_vb);
        if (m_mesh_data->indices_triangulated) {
            ifs->mapBuffer(*m_ctx_ib);
        }
    }

    void copy()
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
            for (int i = 0; i < m_mesh_data->num_indices_triangulated; ++i) {
                indices[i] = (index_t)m_mesh_data->indices_triangulated[i];
            }
            memcpy(m_ctx_ib->data_ptr, buf.cdata(), buf.size());
        }
    }

    void unmap()
    {
        auto ifs = gi::GetGraphicsInterface();
        ifs->unmapBuffer(*m_ctx_vb);
        ifs->unmapBuffer(*m_ctx_ib);
    }
};

class VertexUpdateTaskQueue
{
public:
    typedef VertexUpdateTask task_t;
    typedef std::vector<VertexUpdateTask> tasks_t;

    void push(const VertexUpdateTask& t)
    {
        m_tasks.push_back(t);
    }

    void flush()
    {
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

    bool isFlushing() const { return m_flushing; }

    void clear()
    {
        m_tasks.clear();
    }

private:
    tasks_t m_tasks;
    std::atomic_bool m_flushing = false;
};

std::mutex g_task_mutex[2];
VertexUpdateTaskQueue g_task_queues[2];

} // namespace usdi


extern "C" {

int g_task_index = 0;

usdiAPI int usdiExtGetTaskIndex()
{
    usdiTraceFunc();
    return g_task_index;
}

usdiAPI int usdiExtIncrementTaskIndex()
{
    usdiTraceFunc();
    return g_task_index++;
}

usdiAPI bool usdiExtQueueVertexBufferUpdateTask(const usdi::MeshData *src, usdi::MapContext *ctxVB, usdi::MapContext *ctxIB)
{
    usdiTraceFunc();

    int i = g_task_index & 1;

    if (usdi::g_task_queues[i].isFlushing()) {
        usdiLogWarning("usdiExtQueueVertexBufferUpdateTask(): task queue is flushing!!!\n");
    }
    std::unique_lock<std::mutex> lock(usdi::g_task_mutex[i]);

    if (!src || (!ctxVB && !ctxIB)) { return false; }
    usdi::g_task_queues[i].push(usdi::VertexUpdateTask(src, ctxVB, ctxIB));
    return true;
}

usdiAPI bool usdiExtFlushTaskQueue(int handle)
{
    usdiTraceFunc();
    usdiVTuneScope("usdiExtFlushTaskQueue");

    int i = handle & 1;
    std::unique_lock<std::mutex> lock(usdi::g_task_mutex[i]);

    usdi::g_task_queues[i].flush();
    return true;
}

usdiAPI bool usdiExtClearTaskQueue(int handle)
{
    usdiTraceFunc();
    usdiVTuneScope("usdiExtClearTaskQueue");

    int i = handle & 1;
    std::unique_lock<std::mutex> lock(usdi::g_task_mutex[i]);

    usdi::g_task_queues[i].clear();
    return true;
}

} // extern "C"
