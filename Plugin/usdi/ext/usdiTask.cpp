#include "pch.h"

#ifdef usdiEnableUnityExtension
#include "usdiInternal.h"
#include "usdiTask.h"
#include "usdiUtils.h"
#include "etc/Mono.h"

namespace usdi {

MonoThreadScope::MonoThreadScope()
{
}

MonoThreadScope::~MonoThreadScope()
{
#ifdef usdiEnableMonoThreadGuard
    mono_thread_detach(mono_thread_current());
#endif
}



VertexUpdateCommand::VertexUpdateCommand(const char *dbg_name)
    : m_dbg_name(!dbg_name ? "" : dbg_name)
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

void VertexUpdateCommand::update(const usdi::MeshData *data, void *vb, void *ib)
{
    m_src_points    = data->points;
    m_src_normals   = data->normals;
    m_src_uvs       = data->uvs;
    m_src_tangents  = data->tangents;
    m_src_indices   = data->indices_triangulated;
    m_num_points    = data->num_points;
    m_num_indices   = data->num_indices_triangulated;

    m_ctx_vb.resource = vb;
    m_ctx_ib.resource = ib;

    m_dirty = true;
}

void VertexUpdateCommand::update(const usdi::SubmeshData *data, void *vb, void *ib)
{
    m_src_points    = data->points;
    m_src_normals   = data->normals;
    m_src_uvs       = data->uvs;
    m_src_tangents  = data->tangents;
    m_src_indices   = data->indices;
    m_num_points    = data->num_points;
    m_num_indices   = data->num_points; // num points == num indices on submesh

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
    if (m_src_indices) {
        ifs->mapBuffer(m_ctx_ib);
    }
}

void VertexUpdateCommand::copy()
{
    auto& buf = GetTemporaryBuffer();

    if (m_ctx_vb.data_ptr) {
        if (m_src_uvs) {
            if (m_src_tangents) {
                using vertex_t = vertex_v3n3u2t4;
                vertex_t::source_t src = { m_src_points, m_src_normals, m_src_uvs, m_src_tangents };
                InterleaveBuffered(buf, src, (size_t)m_num_points);
            }
            else {
                using vertex_t = vertex_v3n3u2;
                vertex_t::source_t src = { m_src_points, m_src_normals, m_src_uvs };
                InterleaveBuffered(buf, src, (size_t)m_num_points);
            }
        }
        else {
            using vertex_t = vertex_v3n3;
            vertex_t::source_t src = { m_src_points, m_src_normals };
            InterleaveBuffered(buf, src, (size_t)m_num_points);
        }
        memcpy(m_ctx_vb.data_ptr, buf.data(), buf.size());
    }

    if (m_ctx_ib.data_ptr && m_src_indices) {
        // Unity's mesh index is 16 bit
        // convert 32 bit indices -> 16 bit indices
        using index_t = uint16_t;
        buf.resize(sizeof(index_t) * m_num_indices);
        index_t *indices = (index_t*)buf.data();
        for (size_t i = 0; i < m_num_indices; ++i) {
            indices[i] = (index_t)m_src_indices[i];
        }
        memcpy(m_ctx_ib.data_ptr, buf.data(), buf.size());
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



VertexCommandManager& VertexCommandManager::getInstance()
{
    static VertexCommandManager s_instance;
    return s_instance;
}

VertexUpdateCommand* VertexCommandManager::get(Handle h)
{
    return m_commands.get(h).get();
}

Handle VertexCommandManager::createCommand(const char *dbg_name)
{
    lock_t l(m_mutex_processing);
    return m_commands.push(CommandPtr(new Command(dbg_name)));
}

void VertexCommandManager::destroyCommand(Handle h)
{
    if (h == 0) { return; }

    lock_t l(m_mutex_processing);
    m_commands.pull(h);
}

void VertexCommandManager::update(Handle h, const usdi::MeshData *src, void *vb, void *ib)
{
    if (auto *cmd = get(h)) {
        cmd->update(src, vb, ib);
    }
}

void VertexCommandManager::update(Handle h, const usdi::SubmeshData *src, void *vb, void *ib)
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



tbb::task_group Task::s_task_group;

Task::Task(const std::function<void()>& f, const char *n)
    : m_func(f)
    , m_dbg_name(n)
{
}

void Task::run(bool async)
{
    if (async) {
        m_mutex.lock();
        s_task_group.run([this]() {
            m_func();
            m_mutex.unlock();
        });
    }
    else {
        m_mutex.lock();
        m_func();
        m_mutex.unlock();
    }
}

bool Task::isRunning()
{
    if (m_mutex.try_lock()) {
        m_mutex.unlock();
        return false;
    }
    else {
        return true;
    }
}

void Task::wait()
{
    m_mutex.lock();
    m_mutex.unlock();
}


} // namespace usdi

#endif // usdiEnableUnityExtension
