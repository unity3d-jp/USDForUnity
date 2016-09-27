#include "pch.h"
#include "usdiInternal.h"
#include "usdiSIMD.h"
#include "HandleBasedVector.h"
#include "GraphicsInterface/GraphicsInterface.h"


namespace usdi {

class TempBuffer
{
public:
    TempBuffer() {}
    TempBuffer(const TempBuffer& v) = delete;
    TempBuffer& operator=(const TempBuffer& v) = delete;

    ~TempBuffer()
    {
        clear();
    }

    void* data() { return m_data; }
    size_t size() const { return m_size; }

    void resize(size_t s)
    {
        if (s > m_capacity) {
            clear();
            m_data = AlignedMalloc(s, 0x20);
            m_capacity = s;
        }
        m_size = s;
    }

    void clear()
    {
        AlignedFree(m_data);
        m_data = nullptr;
        m_size = m_capacity = 0;
    }

private:
    void *m_data = nullptr;
    size_t m_size = 0;
    size_t m_capacity = 0;
};

TempBuffer& GetTemporaryBuffer()
{
    static thread_local TempBuffer s_buf;
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

template<class VertexT>
static void WriteVertices(VertexT *dst, const usdi::MeshData& src);
template<class VertexT>
static void WriteVertices(TempBuffer& buf, const usdi::MeshData& src);

#ifdef usdiEnableISPC

template<>
static void WriteVertices(vertex_v3n3 *dst, const usdi::MeshData& src)
{
    ispc::InterleaveVerticesV3N3(
        (ispc::vertex_v3n3*)dst,
        (ispc::float3*)src.points,
        (ispc::float3*)src.normals,
        src.num_points);
}

template<>
static void WriteVertices(vertex_v3n3u2 *dst, const usdi::MeshData& src)
{
    ispc::InterleaveVerticesV3N3U2(
        (ispc::vertex_v3n3u2*)dst,
        (ispc::float3*)src.points,
        (ispc::float3*)src.normals,
        (ispc::float2*)src.uvs,
        src.num_points);
}

template<class VertexT>
static void WriteVertices(TempBuffer& buf, const usdi::MeshData& src)
{
    using vertex_t = VertexT;
    buf.resize(sizeof(vertex_t) * src.num_points);
    WriteVertices((VertexT*)buf.data(), src);
}

#else

template<class VertexT> static inline void WriteVertex(VertexT *dst, const usdi::MeshData& src, int i);

template<> static inline void WriteVertex(vertex_v3n3 *dst, const usdi::MeshData& src, int i)
{
    dst[i].p = src.points[i];
    dst[i].n = src.normals[i];
}

template<> static inline void WriteVertex(vertex_v3n3u2 *dst, const usdi::MeshData& src, int i)
{
    dst[i].p = src.points[i];
    dst[i].n = src.normals[i];
    dst[i].u = src.uvs[i];
}

template<class VertexT>
static void WriteVertices(VertexT *dst, const usdi::MeshData& src)
{
    for (int i = 0; i < src.num_points; ++i) {
        WriteVertex(dst, src, i);
    }
}

template<class VertexT>
static void WriteVertices(TempBuffer& buf, const usdi::MeshData& src)
{
    using vertex_t = VertexT;
    buf.resize(sizeof(vertex_t) * src.num_points);
    vertex_t *dst = (vertex_t*)buf.data();
    WriteVertices((VertexT*)buf.data(), src);
}
#endif


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
                WriteVertices<vertex_v3n3u2>(buf, *m_mesh_data);
            }
            else {
                WriteVertices<vertex_v3n3>(buf, *m_mesh_data);
            }
            memcpy(m_ctx_vb->data_ptr, buf.data(), buf.size());
        }

        if (m_ctx_ib->data_ptr && m_mesh_data->indices_triangulated) {
            // need to convert 32 bit IB -> 16 bit IB...
            using index_t = uint16_t;
            buf.resize(sizeof(index_t) * m_mesh_data->num_indices_triangulated);
            index_t *indices = (index_t*)buf.data();
            for (int i = 0; i < m_mesh_data->num_indices_triangulated; ++i) {
                indices[i] = (index_t)m_mesh_data->indices_triangulated[i];
            }
            memcpy(m_ctx_ib->data_ptr, buf.data(), buf.size());
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

std::mutex g_task_mutex;
VertexUpdateTaskQueue g_task_queues;

} // namespace usdi


extern "C" {

usdiAPI bool usdiExtQueueVertexBufferUpdateTask(const usdi::MeshData *src, usdi::MapContext *ctxVB, usdi::MapContext *ctxIB)
{
    usdiTraceFunc();
    if (usdi::g_task_queues.isFlushing()) {
        usdiLogWarning("usdiExtQueueVertexBufferUpdateTask(): task queue is flushing!!!\n");
    }

    std::unique_lock<std::mutex> lock(usdi::g_task_mutex);

    if (!src || (!ctxVB && !ctxIB)) { return false; }
    usdi::g_task_queues.push(usdi::VertexUpdateTask(src, ctxVB, ctxIB));
    return true;
}

usdiAPI bool usdiExtFlushTaskQueue(usdi::handle_t hq)
{
    usdiTraceFunc();
    usdiVTuneScope("usdiExtFlushTaskQueue");
    std::unique_lock<std::mutex> lock(usdi::g_task_mutex);

    usdi::g_task_queues.flush();
    return true;
}

usdiAPI bool usdiExtClearTaskQueue(usdi::handle_t hq)
{
    usdiTraceFunc();
    usdiVTuneScope("usdiExtClearTaskQueue");
    std::unique_lock<std::mutex> lock(usdi::g_task_mutex);

    usdi::g_task_queues.clear();
    return true;
}

} // extern "C"
