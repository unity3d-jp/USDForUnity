#include <experimental/filesystem>
#include "usdiTests.h"
#include "usdiHelper/usdiHelper.h"


int main(int argc, char *argv[])
{
    namespace fs = std::experimental::filesystem;
    auto cpath = fs::current_path().string();

    AddDLLSearchPath((cpath + "\\plugins\\lib").c_str());
    usdiSetPluginPath((cpath + "\\plugins").c_str());

    if (auto *mod_test = DLLLoad("usdiTests.dll")) {
        void(*RunTests)() = nullptr;
        (void*&)RunTests = DLLGetSymbol(mod_test, "RunTests");
        if (RunTests) {
            RunTests();
        }
    }
}
