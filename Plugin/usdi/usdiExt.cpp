#include "pch.h"
#include "usdiInternal.h"
#include "ext/usdiExtTask.h"


namespace usdi {

int g_vtx_task_index = 0;
VertexUpdateTaskQueue g_vtx_task_queues[2];

TaskGroup g_task_group;

} // namespace usdi


extern "C" {

usdiAPI int usdiExtGetTaskIndex()
{
    usdiTraceFunc();
    return usdi::g_vtx_task_index;
}

usdiAPI int usdiExtIncrementTaskIndex()
{
    usdiTraceFunc();
    return usdi::g_vtx_task_index++;
}

usdiAPI bool usdiExtQueueVertexBufferUpdateTask(const usdi::MeshData *src, usdi::MapContext *ctxVB, usdi::MapContext *ctxIB)
{
    usdiTraceFunc();

    int i = usdi::g_vtx_task_index & 1;

    if (usdi::g_vtx_task_queues[i].isFlushing()) {
        usdiLogWarning("usdiExtQueueVertexBufferUpdateTask(): task queue is flushing!!!\n");
    }
    if (!src || (!ctxVB && !ctxIB)) { return false; }
    usdi::g_vtx_task_queues[i].push(usdi::VertexUpdateTask(src, ctxVB, ctxIB));
    return true;
}

usdiAPI bool usdiExtFlushTaskQueue(int handle)
{
    usdiTraceFunc();
    usdiVTuneScope("usdiExtFlushTaskQueue");

    int i = handle & 1;
    usdi::g_vtx_task_queues[i].flush();
    return true;
}

usdiAPI bool usdiExtClearTaskQueue(int handle)
{
    usdiTraceFunc();
    usdiVTuneScope("usdiExtClearTaskQueue");

    int i = handle & 1;
    usdi::g_vtx_task_queues[i].clear();
    return true;
}



usdiAPI usdi::handle_t usdiExtTaskRun(usdi::TaskFunc func, void *arg)
{
    return usdi::g_task_group.run(func, arg);
}

usdiAPI bool usdiExtTaskIsRunning(usdi::handle_t h)
{
    return usdi::g_task_group.isRunning(h);
}

usdiAPI void usdiExtTaskWait(usdi::handle_t h)
{
    usdi::g_task_group.wait(h);
}

} // extern "C"
