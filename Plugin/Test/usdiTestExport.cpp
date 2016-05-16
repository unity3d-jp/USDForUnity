#include <type_traits>
#include <cstdio>
#include <cmath>
#include "../usdi/usdi.h"

using usdi::float3;

void TestExport(const char *filename)
{
    auto *ctx = usdiCreateContext(filename);
    auto *root = usdiGetRoot(ctx);

    auto *xf = usdiCreateXform(root, "child");
    {
        usdi::XformData data;
        usdi::Time t;
        for (int i = 0; i < 10; ++i) {
            data.position.x += 0.2f;
            t.time += 1.0 / 30.0;
            usdiXformWriteSample(xf, &data, t);
        }
    }

    auto *mesh = usdiCreateMesh(xf, "mesh++");
    {
        float3 vertices[] = {
            { -0.5f, -0.5f, 0.0f },
            {  0.5f, -0.5f, 0.0f },
            {  0.5f,  0.5f, 0.0f },
            { -0.5f,  0.5f, 0.0f },
        };
        int counts[] = { 4 };
        int indices[] = { 0, 1, 2, 3 };

        usdi::MeshData data;
        data.points = vertices;
        data.counts = counts;
        data.indices = indices;
        data.num_points = std::extent<decltype(vertices)>::value;
        data.num_counts = std::extent<decltype(counts)>::value;
        data.num_indices = std::extent<decltype(indices)>::value;

        usdi::Time t;
        for (int i = 0; i < 10; ++i) {
            t.time += 1.0 / 30.0;
            usdiMeshWriteSample(mesh, &data, t);
        }
    }

    usdiWrite(ctx, filename);
    usdiDestroyContext(ctx);
}


int main(int argc, char *argv[])
{
    const char *filename = "test.usda";
    if (argc > 1) {
        filename = argv[1];
    }
    TestExport(filename);
}
