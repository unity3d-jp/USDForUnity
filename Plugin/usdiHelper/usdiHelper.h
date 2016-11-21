#pragma once

#ifdef _WIN32
    #ifndef usdihStaticLink
        #ifdef usdihImpl
            #define usdihAPI __declspec(dllexport)
        #else
            #define usdihAPI __declspec(dllimport)
        #endif
    #else
        #define usdihAPI
    #endif
#else
    #define usdihAPI
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

usdihAPI Platform    GetPlatform();
usdihAPI const char* GetModulePath();
usdihAPI void        AddDLLSearchPath(const char *v);
usdihAPI void        SetEnv(const char *name, const char *value);

usdihAPI module_t DLLLoad(const char *path);
usdihAPI void     DLLUnload(module_t mod);
usdihAPI void*    DLLGetSymbol(module_t mod, const char *name);

usdihAPI void     usdiSetPluginPath(const char *path);

} // extern "C"
