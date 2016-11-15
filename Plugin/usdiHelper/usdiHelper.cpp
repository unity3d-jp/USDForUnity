#ifdef _WIN32
    #include <windows.h>
#else 
    #include <stdlib.h>
    #include <dlfcn.h>
#endif // _WIN32
#include <string>

#define usdihImpl
#include "usdiHelper.h"

extern "C" {

usdihAPI const char* GetModulePath()
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

    return "";
#endif
}

usdihAPI void AddDLLSearchPath(const char *v)
{
#ifdef _WIN32
    std::string path;
    {
        DWORD size = ::GetEnvironmentVariableA("PATH", nullptr, 0);
        if (size > 0) {
            path.resize(size);
            ::GetEnvironmentVariableA("PATH", &path[0], (DWORD)path.size());
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
        ::SetEnvironmentVariableA("PATH", path.c_str());
    }
#else

#endif
}

usdihAPI void SetEnv(const char *name, const char *value)
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

usdihAPI module_t DLLLoad(const char *path) { return ::LoadLibraryA(path); }
usdihAPI void DLLUnload(module_t mod) { ::FreeLibrary((HMODULE)mod); }
usdihAPI void* DLLGetSymbol(module_t mod, const char *name) { return ::GetProcAddress((HMODULE)mod, name); }

#else 

usdihAPI module_t DLLLoad(const char *path) { return ::dlopen(path, RTLD_GLOBAL); }
usdihAPI void DLLUnload(module_t mod) { ::dlclose(mod); }
usdihAPI void* DLLGetSymbol(module_t mod, const char *name) { return ::dlsym(mod, name); }

#endif

usdihAPI void usdiSetPluginPath(const char *path)
{
    SetEnv("PXR_PLUGINPATH_NAME", path);
}

} // extern "C"
