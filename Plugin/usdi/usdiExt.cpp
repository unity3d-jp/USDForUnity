#include "pch.h"
#include "usdiInternal.h"
#include "HandleBasedVector.h"
#include "GraphicsInterface/GraphicsInterface.h"


namespace usdi {

class TaskQueue
{
public:
    typedef std::function<void()> task_t;
    typedef std::vector<task_t> tasks_t;
    typedef tasks_t::iterator iterator_t;

    template<class Lambda>
    void push(const Lambda& lb)
    {
        m_tasks.push_back(lb);
    }

    void flush()
    {
        tbb::parallel_for_each(m_tasks, [](auto& task) { task(); });
        m_tasks.clear();
    }

private:
    tasks_t m_tasks;
};
typedef std::shared_ptr<TaskQueue> TaskQueuePtr;

HandleBasedVector<TaskQueuePtr> g_task_queues;


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

static bool UpdateVertexBuffer(const usdi::MeshData& src, void *vb, void *ib)
{
    auto *gi = gi::GetGraphicsInterface();
    if (!gi) { return false; }

    auto& buf = GetTemporaryBuffer();

    if (vb) {
        if (src.uvs) { WriteVertices<vertex_v3n3u2>(src, buf); }
        else { WriteVertices<vertex_v3n3>(src, buf); }
        gi->writeBuffer(vb, &buf[0], buf.size(), gi::BufferType::Vertex);
    }

    if (ib) {
        // need to convert 32 bit IB -> 16 bit IB...
        using index_t = uint16_t;
        buf.resize(sizeof(index_t) * src.num_indices_triangulated);
        index_t *indices = (index_t*)&buf[0];

        for (int i = 0; i < src.num_indices_triangulated; ++i) {
            indices[i] = (index_t)src.indices_triangulated[i];
        }
        gi->writeBuffer(vb, &buf[0], buf.size(), gi::BufferType::Index);
    }

    return true;
}

} // namespace usdi


extern "C" {

usdiAPI usdi::handle_t usdiExtCreateTaskQueue()
{
    usdiTraceFunc();
    return usdi::g_task_queues.push(new usdi::TaskQueue());
}

usdiAPI bool usdiExtDestroyTaskQueue(usdi::handle_t hq)
{
    usdiTraceFunc();
    if (!usdi::g_task_queues.valid(hq)) {
        return false;
    }
    usdi::g_task_queues.pull(hq);
    return true;
}

usdiAPI bool usdiExtQueueVertexBufferUpdateTask(usdi::handle_t hq, const usdi::MeshData *src, void *vb, void *ib)
{
    usdiTraceFunc();
    if (!src || (!vb && !ib) || !usdi::g_task_queues.valid(hq)) { return false; }
    usdi::g_task_queues.get(hq).push([=]() {
        usdiLogTrace("vertex buffer update task\n");
        usdi::UpdateVertexBuffer(*src, vb, ib);
    });
    return true;
}

usdiAPI bool usdiExtFlushTaskQueue(usdi::handle_t hq)
{
    usdiTraceFunc();
    if (!usdi::g_task_queues.valid(hq)) { return false; }
    usdi::g_task_queues.get(hq).flush();
    return true;
}

} // extern "C"
