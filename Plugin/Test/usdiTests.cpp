#define testsImpl
#include "usdiTests.h"

void TestExportMesh(const char *filename);
void TestExportHighMesh(const char *filename);
void TestExportReference(const char *filename, const char *flatten);
bool TestImport(const char *path);

extern "C" {

testsAPI void TestMain(int argc, char *argv[])
{
    TestExportMesh("TestExport.usda");
    TestExportMesh("TestExport.usdc");
    TestExportHighMesh("HighMesh.usda");
    TestExportHighMesh("HighMesh.usdc");
    TestExportReference("TestReference.usda", "Flatten.usda");

    TestImport("TestExport.usda");
    TestImport("TestReference.usda");
}

} // extern "C"
