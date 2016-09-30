#include "pch.h"
#include "usdiInternal.h"
#include "ext/usdiExtTask.h"


namespace usdi {

VertexUpdateTaskManager g_vtx_task_manager;
TaskManager g_task_manager;

} // namespace usdi


extern "C" {

usdiAPI void usdiExtVtxTaskQueue(const usdi::MeshData *src, usdi::MapContext *ctxVB, usdi::MapContext *ctxIB)
{
    usdiTraceFunc();

    if (!src || (!ctxVB && !ctxIB)) { return; }
    return usdi::g_vtx_task_manager.queue(usdi::VertexUpdateTask(src, ctxVB, ctxIB));
}

usdiAPI void usdiExtVtxTaskEndQueing()
{
    usdi::g_vtx_task_manager.endQueing();
}

usdiAPI void usdiExtVtxTaskFlush()
{
    usdiTraceFunc();
    usdiVTuneScope("usdiExtFlushTaskQueue");

    usdi::g_vtx_task_manager.flush();
}

usdiAPI void usdiExtVtxTaskClear()
{
    usdiTraceFunc();
    usdiVTuneScope("usdiExtFlushTaskQueue");

    usdi::g_vtx_task_manager.clear();
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

