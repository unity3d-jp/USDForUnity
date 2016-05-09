#pragma once

#define usdeCLinkage extern "C"
#ifdef _WIN32
    #ifndef usdeStaticLink
        #ifdef usdeImpl
            #define usdeExport __declspec(dllexport)
        #else
            #define usdeExport __declspec(dllimport)
        #endif
    #else
        #define usdeExport
    #endif
#else
    #define usdeExport
#endif
