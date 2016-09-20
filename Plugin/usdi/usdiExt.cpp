#include "pch.h"
#include "usdiInternal.h"
#include "usdiSIMD.h"
#include "HandleBasedVector.h"
#include "GraphicsInterface/GraphicsInterface.h"


namespace usdi {

std::vector<char>& GetTemporaryBuffer()
{
    static thread_local std::vector<char> s_buf;
    return s_buf;
}


struct vertex_v3n3
{
    float3 p;
    float3 n;
};

struct vertex_v3n3u2
{
    float3 p;
    float3 n;
    float2 u;
};

template<class VertexT> static inline void WriteVertex(const usdi::MeshData& src, VertexT *dst, int i);

template<> static inline void WriteVertex(const usdi::MeshData& src, vertex_v3n3 *dst, int i)
{
    dst[i].p = src.points[i];
    dst[i].n = src.normals[i];
}

template<> static inline void WriteVertex(const usdi::MeshData& src, vertex_v3n3u2 *dst, int i)
{
    dst[i].p = src.points[i];
    dst[i].n = src.normals[i];
    dst[i].u = src.uvs[i];
}

template<class VertexT>
static void WriteVertices(const usdi::MeshData& src, std::vector<char>& buf)
{
    using vertex_t = VertexT;

    buf.resize(sizeof(vertex_t) * src.num_points);
    vertex_t *dst = (vertex_t*)&buf[0];

    for (int i = 0; i < src.num_points; ++i) {
        WriteVertex(src, dst, i);
    }
}


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
        if (m_mesh_data->indices) {
            ifs->mapBuffer(*m_ctx_ib);
        }
    }

    void copy()
    {
        auto *ifs = gi::GetGraphicsInterface();

        auto& buf = GetTemporaryBuffer();

        if (m_ctx_vb->data_ptr) {
            if (m_mesh_data->uvs) { WriteVertices<vertex_v3n3u2>(*m_mesh_data, buf); }
            else { WriteVertices<vertex_v3n3>(*m_mesh_data, buf); }
            memcpy(m_ctx_vb->data_ptr, &buf[0], buf.size());
        }

        if (m_ctx_ib->data_ptr) {
            // need to convert 32 bit IB -> 16 bit IB...
            using index_t = uint16_t;
            buf.resize(sizeof(index_t) * m_mesh_data->num_indices_triangulated);
            index_t *indices = (index_t*)&buf[0];

            for (int i = 0; i < m_mesh_data->num_indices_triangulated; ++i) {
                indices[i] = (index_t)m_mesh_data->indices_triangulated[i];
            }
            memcpy(m_ctx_ib->data_ptr, &buf[0], buf.size());
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
        for (auto& t : m_tasks) { t.map(); }
        tbb::parallel_for_each(m_tasks, [](auto& t) { t.copy(); });
        for (auto& t : m_tasks) { t.unmap(); }
        m_tasks.clear();
    }

private:
    tasks_t m_tasks;
};

std::mutex g_task_mutex;
VertexUpdateTaskQueue g_task_queues;

} // namespace usdi


extern "C" {

usdiAPI bool usdiExtQueueVertexBufferUpdateTask(const usdi::MeshData *src, usdi::MapContext *ctxVB, usdi::MapContext *ctxIB)
{
    usdiTraceFunc();
    std::unique_lock<std::mutex> lock(usdi::g_task_mutex);

    if (!src || (!ctxVB && !ctxIB)) { return false; }
    usdi::g_task_queues.push(usdi::VertexUpdateTask(src, ctxVB, ctxIB));
    return true;
}

usdiAPI bool usdiExtFlushTaskQueue(usdi::handle_t hq)
{
    usdiTraceFunc();
    std::unique_lock<std::mutex> lock(usdi::g_task_mutex);

    usdi::g_task_queues.flush();
    return true;
}

} // extern "C"
