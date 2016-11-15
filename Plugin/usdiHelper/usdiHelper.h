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

using module_t = void*;

extern "C" {

usdihAPI const char* GetModulePath();
usdihAPI void        AddDLLSearchPath(const char *v);
usdihAPI void        SetEnv(const char *name, const char *value);

usdihAPI module_t DLLLoad(const char *path);
usdihAPI void     DLLUnload(module_t mod);
usdihAPI void*    DLLGetSymbol(module_t mod, const char *name);

usdihAPI void     usdiSetPluginPath(const char *path);

} // extern "C"
