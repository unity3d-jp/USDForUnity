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
        if(!path.empty()) { path += ";"; }
        auto pos = path.size();
        path += v;
        for (size_t i = pos; i < path.size(); ++i) {
            char& c = path[i];
            if (c == '/') { c = '\\'; }
        }
        ::SetEnvironmentVariableA(LIBRARY_PATH, path.c_str());
    }
#else
    std::string path;
    if (auto path_ = ::getenv(LIBRARY_PATH)) {
        path = path_;
    }
    if (path.find(v) == std::string::npos) {
        if(!path.empty()) { path += ":"; }
        auto pos = path.size();
        path += v;
        for (size_t i = pos; i < path.size(); ++i) {
            char& c = path[i];
            if (c == '\\') { c = '/'; }
        }
        ::setenv(LIBRARY_PATH, path.c_str(), 1);
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

rtAPI module_t  DLLLoad(const char *path) { return ::LoadLibraryA(path); }
rtAPI void      DLLUnload(module_t mod) { ::FreeLibrary((HMODULE)mod); }
rtAPI void*     DLLGetSymbol(module_t mod, const char *name) { return ::GetProcAddress((HMODULE)mod, name); }
rtAPI module_t  DLLGetHandle(const char *modname) { return ::GetModuleHandleA(modname); }

#else

rtAPI module_t  DLLLoad(const char *path) { return ::dlopen(path, RTLD_LAZY); }
rtAPI void      DLLUnload(module_t mod) { ::dlclose(mod); }
rtAPI void*     DLLGetSymbol(module_t mod, const char *name)
{
    return ::dlsym(mod, name);
}

rtAPI module_t DLLGetHandle(const char *modname)
{
#ifdef __APPLE__

    for (int i = (int)_dyld_image_count(); i >= 0; i--) {
        auto *path = _dyld_get_image_name(i);
        if (strstr(path, modname)) {
            return dlopen(path, RTLD_LAZY);
        }
    }

#else

    auto *mod = dlopen(nullptr, RTLD_LAZY);
    link_map *it = nullptr;
    dlinfo(mod, RTLD_DI_LINKMAP, &it);
    while (it) {
        if (strstr(it->l_name, modname)) {
            return dlopen(it->l_name, RTLD_LAZY);
        }
        it = it->l_next;
    }

#endif
    return nullptr;
}

#endif

rtAPI void usdiSetPluginPath(const char *path_)
{
    std::string path = path_;
    for (char& c : path) {
#if _WIN32
        if (c == '/') { c = '\\'; }
#else
        if (c == '\\') { c = '/'; }
#endif
    }
    SetEnv("PXR_PLUGINPATH_NAME", path.c_str());
}

} // extern "C"
