#include <cstdio>
#include "../usdi/usdi.h"


void TestExport()
{
    auto *ctx = usdiCreateContext("hoge.usd");
    auto *root = usdiGetRoot(ctx);

    auto *xf = usdiAsXform(root);
    {
        usdi::XformData data;
        usdi::Time t;
        for (int i = 0; i < 10; ++i) {
            data.position.x += 0.1f;
            t.time += 1.0 / 30.0;
        }
        usdiXformWriteSample(xf, &data, t);
    }

    usdiWrite(ctx, "hoge.usd");
    usdiDestroyContext(ctx);
}


int main(int argc, char *argv[])
{
    TestExport();
}
