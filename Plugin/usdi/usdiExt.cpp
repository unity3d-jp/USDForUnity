#include "pch.h"
#include "usdiInternal.h"
#include "ext/usdiExtTask.h"


namespace usdi {

VertexCommandManager g_vtx_task_manager;
TaskManager g_task_manager;

} // namespace usdi


extern "C" {

usdiAPI usdi::handle_t usdiExtVtxCmdCreate(const char *dbg_name)
{
    usdiTraceFunc();
    return usdi::g_vtx_task_manager.createCommand(dbg_name);
}

usdiAPI void usdiExtVtxCmdDestroy(usdi::handle_t h)
{
    usdiTraceFunc();
    usdi::g_vtx_task_manager.destroyCommand(h);
}

usdiAPI void usdiExtVtxCmdUpdate(usdi::handle_t h, const usdi::MeshData *src, void *vb, void *ib)
{
    usdiTraceFunc();
    usdi::g_vtx_task_manager.update(h, src, vb, ib);
}

usdiAPI void usdiExtVtxCmdProcess()
{
    usdiTraceFunc();
    usdiVTuneScope("usdiExtVtxCmdKick");
    usdi::g_vtx_task_manager.process();
}

usdiAPI void usdiExtVtxCmdWait()
{
    usdiTraceFunc();
    usdiVTuneScope("usdiExtVtxCmdKick");
    usdi::g_vtx_task_manager.wait();
}




usdiAPI usdi::handle_t usdiExtTaskCreate(usdi::TaskFunc func, void *arg, const char *name)
{
    usdiTraceFunc();
    return usdi::g_task_manager.createTask(func, arg, name);
}

usdiAPI void usdiExtTaskDestroy(usdi::handle_t h)
{
    usdiTraceFunc();
    usdi::g_task_manager.destroyTask(h);
}

usdiAPI void usdiExtTaskRun(usdi::handle_t h)
{
    usdiTraceFunc();
    usdi::g_task_manager.run(h);
}

usdiAPI bool usdiExtTaskIsRunning(usdi::handle_t h)
{
    usdiTraceFunc();

    return usdi::g_task_manager.isRunning(h);
}

usdiAPI void usdiExtTaskWait(usdi::handle_t h)
{
    usdiTraceFunc();

    usdi::g_task_manager.wait(h);
}

} // extern "C"

