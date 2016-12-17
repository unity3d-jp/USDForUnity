#ifdef _WIN32
    #include <windows.h>
#else 
    #include <stdlib.h>
    #include <dlfcn.h>
    #include <link.h>
#endif
#include <string>


#define rtImpl
