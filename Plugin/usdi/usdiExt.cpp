#include "pch.h"
#include "usdiInternal.h"
#include "usdiUtils.h"
#include "GraphicsInterface/GraphicsInterface.h"
#include "ext/usdiExtTask.h"


namespace usdi {

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
