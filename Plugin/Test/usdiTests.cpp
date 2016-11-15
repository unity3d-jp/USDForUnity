#define testsImpl
#include "usdiTests.h"

void TestExportMesh(const char *filename);
void TestExportHighMesh(const char *filename);
bool TestImport(const char *path);

extern "C" {

    testsAPI void RunTests()
{
    TestExportMesh("TestExport.usda");
    TestExportMesh("TestExport.usdc");
    TestExportHighMesh("HighMesh.usda");
    TestExportHighMesh("HighMesh.usdc");

    TestImport("TestExport.usda");
    TestImport("TestExport.usdc");
}

} // extern "C"
