#pragma once

#ifdef _WIN32
    #ifndef rtStaticLink
        #ifdef rtImpl
            #define rtAPI __declspec(dllexport)
        #else
            #define rtAPI __declspec(dllimport)
        #endif
    #else
        #define rtAPI
    #endif
#else
    #define rtAPI
#endif

enum class Platform
{
    Unknown,
    // any 32bit architectures are treated as "Unknown" :)
    Windows_x86_64,
    Linux_x86_64,
    Mac_x86_64,
    Android_ARM64,
    iOS_ARM64,
    PS4,
};

using module_t = void*;

extern "C" {

rtAPI Platform    GetPlatform();
rtAPI const char* GetModulePath();
rtAPI void        AddDLLSearchPath(const char *v);
rtAPI void        SetEnv(const char *name, const char *value);

rtAPI module_t DLLLoad(const char *path);
rtAPI void     DLLUnload(module_t mod);
rtAPI void*    DLLGetSymbol(module_t mod, const char *name);
rtAPI module_t DLLGetHandle(const char *modname);

rtAPI void     usdiSetPluginPath(const char *path);

} // extern "C"
