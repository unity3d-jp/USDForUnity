#pragma once

#include "GraphicsInterface/GraphicsInterface.h"

namespace usdi {

struct MapContext : gi::MapContext {};

class VertexUpdateTask
{

public:
    VertexUpdateTask(const usdi::MeshData *mesh_data, MapContext *ctx_vb, MapContext *ctx_ib);
    ~VertexUpdateTask();

    void map();
    void copy();
    void unmap();

private:
    const usdi::MeshData *m_mesh_data;
    MapContext *m_ctx_vb;
    MapContext *m_ctx_ib;
};

class VertexUpdateTaskQueue
{
public:
    typedef VertexUpdateTask task_t;
    typedef std::vector<VertexUpdateTask> tasks_t;

    void push(const VertexUpdateTask& t);
    void flush();
    bool isFlushing() const;
    void clear();

private:
    tasks_t m_tasks;
    std::atomic_bool m_flushing = false;
};


} // namespace usdi
