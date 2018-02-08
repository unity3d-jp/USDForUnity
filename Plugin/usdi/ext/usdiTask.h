#pragma once

#include "usdiExt.h"
#include "MeshUtils/muHandleBasedVector.h"

namespace usdi {

class Task
{
public:
    Task(const std::function<void()>& f, const char *n = "");
    ~Task();
    void run(bool async = true);
    bool isRunning();
    void wait();

private:
    std::string m_dbg_name;
    std::function<void()> m_func;
    tbb::spin_mutex m_mutex; // must be non-recursive mutex
};

} // namespace usdi
