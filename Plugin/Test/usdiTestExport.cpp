#include <cstdio>
#include "../usdi/usdi.h"


void TestExport()
{
    auto *ctx = usdiCreateContext("hoge.usd");
    usdiWrite(ctx, "hoge.usd");
    usdiDestroyContext(ctx);
}


int main(int argc, char *argv[])
{
    TestExport();
}
