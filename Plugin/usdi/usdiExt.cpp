#include "pch.h"
#include "usdiInternal.h"
#include "ext/usdiExtTask.h"

#include "usdiAttribute.h"
#include "usdiSchema.h"
#include "usdiXform.h"
#include "usdiCamera.h"
#include "usdiMesh.h"
#include "usdiPoints.h"
#include "usdiContext.h"


namespace usdi {

VertexCommandManager g_vtx_task_manager;
TaskManager g_task_manager;

} // namespace usdi


extern "C" {

usdiAPI usdi::handle_t usdiVtxCmdCreate(const char *dbg_name)
{
    usdiTraceFunc();
    return usdi::g_vtx_task_manager.createCommand(dbg_name);
}

usdiAPI void usdiVtxCmdDestroy(usdi::handle_t h)
{
    usdiTraceFunc();
    usdi::g_vtx_task_manager.destroyCommand(h);
}

usdiAPI void usdiVtxCmdUpdate(usdi::handle_t h, const usdi::MeshData *src, void *vb, void *ib)
{
    usdiTraceFunc();
    usdi::g_vtx_task_manager.update(h, src, vb, ib);
}

usdiAPI void usdiVtxCmdProcess()
{
    usdiTraceFunc();
    usdiVTuneScope("usdiVtxCmdKick");
    usdi::g_vtx_task_manager.process();
}

usdiAPI void usdiVtxCmdWait()
{
    usdiTraceFunc();
    usdiVTuneScope("usdiVtxCmdKick");
    usdi::g_vtx_task_manager.wait();
}




usdiAPI usdi::handle_t usdiTaskCreate(usdi::TaskFunc func, void *arg, const char *name)
{
    usdiTraceFunc();
    return usdi::g_task_manager.createTask([=]() { func(arg); }, name);
}

usdiAPI void usdiTaskDestroy(usdi::handle_t h)
{
    usdiTraceFunc();
    usdi::g_task_manager.destroyTask(h);
}

usdiAPI void usdiTaskRun(usdi::handle_t h)
{
    usdiTraceFunc();
    usdi::g_task_manager.run(h);
}

usdiAPI bool usdiTaskIsRunning(usdi::handle_t h)
{
    usdiTraceFunc();
    return usdi::g_task_manager.isRunning(h);
}

usdiAPI void usdiTaskWait(usdi::handle_t h)
{
    usdiTraceFunc();
    usdi::g_task_manager.wait(h);
}

usdiAPI usdi::handle_t usdiTaskMeshReadSample(usdi::Mesh *mesh, usdi::MeshData *dst, const usdi::Time *t)
{
    usdiTraceFunc();
    return usdi::g_task_manager.createTask([=]() {
        return mesh->readSample(*dst, *t, true);
    });
}

usdiAPI usdi::handle_t usdiTaskPointsReadSample(usdi::Points *points, usdi::PointsData *dst, const usdi::Time *t)
{
    return usdi::g_task_manager.createTask([=]() {
        return points->readSample(*dst, *t, true);
    });
}

} // extern "C"

