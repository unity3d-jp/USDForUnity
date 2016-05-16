#include <cstdio>
#include <cmath>
#include "../usdi/usdi.h"


void TestExport(const char *filename)
{
    auto *ctx = usdiCreateContext(filename);
    auto *root = usdiGetRoot(ctx);

    auto *xf = usdiCreateXform(root, "child");
    {
        usdi::XformData data;
        usdi::Time t;
        for (int i = 0; i < 10; ++i) {
            data.position.x = std::pow(1.1f, i);
            t.time += 1.0 / 30.0;
        }
        usdiXformWriteSample(xf, &data, t);
    }

    usdiWrite(ctx, filename);
    usdiDestroyContext(ctx);
}


int main(int argc, char *argv[])
{
    const char *filename = "hoge.usd";
    if (argc > 1) {
        filename = argv[1];
    }
    TestExport(filename);
}
