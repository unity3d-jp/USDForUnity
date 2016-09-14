#include "pch.h"
#include "usdiInternal.h"
#include "HandleBasedVector.h"


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
    if (usdi::g_task_queues.valid(hq)) {
        usdi::g_task_queues.pull(hq);
        return true;
    }
    return false;
}

usdiAPI bool usdiExtQueueVertexBufferUpdateTask(usdi::handle_t hq, const usdi::MeshData *src, void *vb, void *ib)
{
    usdiTraceFunc();
    if (!src || (!vb && !ib) || !usdi::g_task_queues.valid(hq)) { return false; }
    usdi::g_task_queues.get(hq).push([=]() {
        // todo
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
