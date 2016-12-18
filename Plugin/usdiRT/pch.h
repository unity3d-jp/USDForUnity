#pragma once

#define rtImpl

#ifdef _WIN32
    #include <windows.h>
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

