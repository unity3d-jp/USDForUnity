#ifdef _WIN32
    #include <windows.h>
    #define DLLEXPORT __declspec(dllexport)
#else 
    #define DLLEXPORT
#endif // _WIN32
#include <string>

extern "C" DLLEXPORT void AddDLLSearchPath()
{
    static bool s_initialized = false;
    if (!s_initialized) {
        s_initialized = true;

#ifdef _WIN32
        std::string path;
        path.resize(1024 * 128);
        DWORD ret = ::GetEnvironmentVariableA("PATH", &path[0], (DWORD)path.size());
        path.resize(ret);

        char path_to_this_module[MAX_PATH + 1];
        HMODULE mod = 0;
        ::GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)&AddDLLSearchPath, &mod);
        DWORD size = ::GetModuleFileNameA(mod, path_to_this_module, sizeof(path_to_this_module));
        for (int i = size - 1; i >= 0; --i) {
            if (path_to_this_module[i] == '\\') {
                path_to_this_module[i] = '\0';
                break;
            }
        }
        path += ";";
        path += path_to_this_module;

        ::SetEnvironmentVariableA("PATH", path.c_str());
#else

#endif
    }
}
