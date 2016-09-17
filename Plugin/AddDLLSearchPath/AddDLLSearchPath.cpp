#ifdef _WIN32
    #include <windows.h>
    #define DLLEXPORT __declspec(dllexport)
#else 
    #include <stdlib.h>
    #define DLLEXPORT
#endif // _WIN32
#include <string>
#include <mutex>


extern "C" DLLEXPORT const char* GetModulePath()
{
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
}

extern "C" DLLEXPORT void AddDLLSearchPath(const char *v)
{
#ifdef _WIN32
    std::string path;
    {
        DWORD size = ::GetEnvironmentVariableA("PATH", nullptr, 0);
        path.resize(size);
        ::GetEnvironmentVariableA("PATH", &path[0], (DWORD)path.size());
        path.pop_back(); // delete last '\0'
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

extern "C" DLLEXPORT void SetEnv(const char *name, const char *value)
{
#ifdef _WIN32
    // get/setenv() and Set/GetEnvironmentVariable() is *not* compatible.
    // set both to make sure.
    std::string tmp = name;
    tmp += "=";
    tmp += value;
    ::_putenv(tmp.c_str());
    ::SetEnvironmentVariableA(name, value);
#else
    ::setenv(name, value, 1);
#endif
}

