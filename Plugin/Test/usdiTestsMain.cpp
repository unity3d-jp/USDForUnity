#include "usdiTests.h"
#include "usdiRT/usdiRT.h"
#include <string>
#ifdef _WIN32
    #include <experimental/filesystem>
#else
    #include <boost/filesystem.hpp>
#endif

#ifdef _WIN32
    #define TestModule "usdiTests.dll"
#else
    #define TestModule "usdiTests"
#endif

int main(int argc, char *argv[])
{
    namespace fs =
#ifdef _WIN32
        std::experimental::filesystem;
#else
        boost::filesystem;
#endif
    auto cpath = fs::current_path().string();
    AddDLLSearchPath((cpath + "/plugins/lib").c_str());
    usdiSetPluginPath((cpath + "/plugins").c_str());

    if (auto *mod_test = DLLLoad(TestModule)) {
        void(*TestMain)(int, char**) = nullptr;
        (void*&)TestMain = DLLGetSymbol(mod_test, "TestMain");
        printf("TestMain = %p\n", TestMain);
        if (TestMain) {
            TestMain(argc, argv);
        }
    }
    else {
        printf("failed to load %s\n", TestModule);
    }
}
