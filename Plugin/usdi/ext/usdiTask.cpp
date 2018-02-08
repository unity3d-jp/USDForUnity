#include "pch.h"

#include "usdiInternal.h"
#include "usdiTask.h"
#include "usdiUtils.h"

namespace usdi {

Task::Task(const std::function<void()>& f, const char *n)
    : m_dbg_name(n)
    , m_func(f)
{
}

Task::~Task()
{
}

void Task::run(bool async)
{
    if (async) {
        m_mutex.lock();
        launch([this]() {
            m_func();
            m_mutex.unlock();
        });
    }
    else {
        m_func();
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

