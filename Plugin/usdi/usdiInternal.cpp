#include "pch.h"
#include "usdiInternal.h"

namespace usdi {

void LogImpl(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    fflush(stdout);
}

TraceFuncImpl::TraceFuncImpl(const char *func)
    : m_func(func)
{
    usdiTrace("%s enter\n", m_func);
}

TraceFuncImpl::~TraceFuncImpl()
{
    usdiTrace("%s leave\n", m_func);
}

} // namespace usdi
