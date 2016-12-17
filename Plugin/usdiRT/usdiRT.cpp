#include "pch.h"
#include "usdiRT.h"
#include "../usdi/etc/Platform.h"

extern "C" {

rtAPI Platform GetPlatform()
{
#if defined(__Windows_x86_64__)
    return Platform::Windows_x86_64;
#elif defined(__Linux_x86_64__)
    return Platform::Linux_x86_64;
#elif defined(__Mac_x86_64__)
    return Platform::Mac_x86_64;
#elif defined(__Android_ARM64__)
    return Platform::Android_ARM64;
#elif defined(__iOS_ARM64__)
    return Platform::iOS_ARM64;
#elif defined(__PS4__)
    return Platform::PS4;
#endif
}

rtAPI const char* GetModulePath()
{
#ifdef _WIN32
    static char s_path[MAX_PATH + 1];
    if (s_path[0] == 0) {
        HMODULE mod = 0;
        ::GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)&GetModulePath, &mod);
        DWORD size = ::GetModuleFileNameA(mod, s_path, sizeof(s_path));
        for (int i = size - 1; i >= 0; --i) {
            if (s_path[i] == '\\') {
                s_path[i] = '\0';
                break;
            }
        }
    }
    return s_path;
#else
    static Dl_info s_info;
    if (!s_info.dli_fname) {
        dladdr((const void*)&GetModulePath, &s_info);
    }
    return s_info.dli_fname;
#endif
}

rtAPI void AddDLLSearchPath(const char *v)
{
#ifdef _WIN32
    std::string path;
    {
        DWORD size = ::GetEnvironmentVariableA(LIBRARY_PATH, nullptr, 0);
        if (size > 0) {
            path.resize(size);
            ::GetEnvironmentVariableA(LIBRARY_PATH, &path[0], (DWORD)path.size());
            path.pop_back(); // delete last '\0'
        }
    }
    if (path.find(v) == std::string::npos) {
        path += ";";
        auto pos = path.size();
        path += v;
        for (size_t i = pos; i < path.size(); ++i) {
            if (path[i] == '/') {
                path[i] = '\\';
            }
        }
        ::SetEnvironmentVariableA(LIBRARY_PATH, path.c_str());
    }
#else
    std::string path = ::getenv(LIBRARY_PATH);
    if (path.find(v) == std::string::npos) {
        path += ":";
        auto pos = path.size();
        path += v;
        for (size_t i = pos; i < path.size(); ++i) {
            if (path[i] == '\\') {
                path[i] = '/';
            }
        }
        ::setenv(LIBRARY_PATH, path.c_str(), 0);
    }
#endif
}

rtAPI void SetEnv(const char *name, const char *value)
{
#ifdef _WIN32
    // get/setenv() and Set/GetEnvironmentVariable() is *not* compatible.
    // set both to make sure.
    ::_putenv_s(name, value);
    ::SetEnvironmentVariableA(name, value);
#else
    ::setenv(name, value, 1);
#endif
}

#ifdef _WIN32

rtAPI module_t DLLLoad(const char *path) { return ::LoadLibraryA(path); }
rtAPI void DLLUnload(module_t mod) { ::FreeLibrary((HMODULE)mod); }
rtAPI void* DLLGetSymbol(module_t mod, const char *name) { return ::GetProcAddress((HMODULE)mod, name); }

#else 

rtAPI module_t DLLLoad(const char *path) { return ::dlopen(path, RTLD_GLOBAL); }
rtAPI void DLLUnload(module_t mod) { ::dlclose(mod); }
rtAPI void* DLLGetSymbol(module_t mod, const char *name) { return ::dlsym(mod, name); }

#endif

rtAPI void usdiSetPluginPath(const char *path)
{
    SetEnv("PXR_PLUGINPATH_NAME", path);
}

} // extern "C"
