#pragma once
#include "USDImporter.h"

#define usdiLog(...) usdi::LogImpl(__FUNCTION__,  __VA_ARGS__)

namespace usdi {

void LogImpl(const char *function, const char *format, ...);

} // namespace usdi
