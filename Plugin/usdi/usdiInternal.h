#pragma once
#include "usdi.h"

#define usdiLog(...) usdi::LogImpl(__VA_ARGS__)

namespace usdi {

void LogImpl(const char *format, ...);

} // namespace usdi
