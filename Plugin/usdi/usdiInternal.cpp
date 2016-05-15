#include "pch.h"
#include "usdiInternal.h"

namespace usdi {

void LogImpl(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

} // namespace usdi
