#pragma once

#define rtImpl

#ifdef _WIN32
    #define NOMINMAX
    #include <windows.h>
    #pragma warning(disable:4996)
#else 
    #include <dlfcn.h>
    #ifdef __APPLE__
        #include <mach-o/dyld.h>
    #else
        #include <link.h>
    #endif
#endif
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

