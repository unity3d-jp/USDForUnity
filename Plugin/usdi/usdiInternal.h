#pragma once

#define usdiImpl

#define usdiLog(...) usdi::LogImpl(__VA_ARGS__)
#ifdef usdiMaster
    #define usdiTrace(...)
#else
    #define usdiTrace(...) usdi::LogImpl(__VA_ARGS__)
#endif

namespace usdi {

void LogImpl(const char *format, ...);

} // namespace usdi

#include "usdi.h"
