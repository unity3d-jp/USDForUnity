#include <type_traits>
#include <cstdio>
#include <cmath>
#include <vector>
#include <algorithm>
#include "../usdi/usdi.h"

using usdi::float2;
using usdi::float3;
using usdi::float4;
using usdi::quatf;

const int WaveMeshResolution = 256;

static void GenerateWaveMeshTopology(std::vector<int>& counts, std::vector<int>& indices)
{
    const int num_faces = (WaveMeshResolution - 1) * (WaveMeshResolution - 1);
    const int num_indices = num_faces * 4;

    counts.resize(num_faces);
    indices.resize(num_indices);

    for (int iy = 0; iy < WaveMeshResolution - 1; ++iy) {
        for (int ix = 0; ix < WaveMeshResolution - 1; ++ix) {
            int i = (WaveMeshResolution-1)*iy + ix;
            counts[i] = 4;
            indices[i * 4 + 0] = WaveMeshResolution*iy + ix;
            indices[i * 4 + 1] = WaveMeshResolution*iy + (ix + 1);
            indices[i * 4 + 2] = WaveMeshResolution*(iy + 1) + (ix + 1);
            indices[i * 4 + 3] = WaveMeshResolution*(iy + 1) + ix;
        }
    }
}

static void GenerateWaveMesh(std::vector<float3>& vertices, usdi::Time t)
{
    const float hald_res = (float)(WaveMeshResolution / 2);
    int num_vertices = WaveMeshResolution * WaveMeshResolution;

    vertices.resize(num_vertices);

    for (int iy = 0; iy < WaveMeshResolution; ++iy) {
        for (int ix = 0; ix < WaveMeshResolution; ++ix) {
            int i = WaveMeshResolution*iy + ix;
            float2 pos = { (float(ix) - hald_res) / hald_res, (float(iy) - hald_res) / hald_res };
            float d = std::sqrt(pos.x*pos.x + pos.y*pos.y);

            float3& v = vertices[i];
            v.x = pos.x;
            v.z = pos.y;
            v.y = std::sin(d * 10.0f + t * 5.0f) * std::max<float>(1.0 - d, 0.0f);
        }
    }
}


void TestExportHighMesh(const char *filename)
{
    auto *ctx = usdiCreateContext();
    usdiCreateStage(ctx, filename);
    auto *root = usdiGetRoot(ctx);

    auto *xf = usdiCreateXform(ctx, root, "Root");
    {
        usdi::XformData data;
        usdi::Time t = 0.0;
        for (int i = 0; i < 150; ++i) {
            data.position.x += 0.2f;
            t += 1.0 / 30.0;
            usdiXformWriteSample(xf, &data, t);
        }
    }

    auto *mesh = usdiCreateMesh(ctx, xf, "WaveMesh");
    {
        std::vector<float3> vertices;
        std::vector<int> counts;
        std::vector<int> indices;

        usdi::Time t = 0.0;
        GenerateWaveMeshTopology(counts, indices);
        GenerateWaveMesh(vertices, t);

        usdi::MeshData data;
        data.points = vertices.data();
        data.counts = counts.data();
        data.indices = indices.data();
        data.num_points = vertices.size();
        data.num_counts = counts.size();
        data.num_indices = indices.size();

        for (int i = 0; i < 5; ++i) {
            t += 1.0 / 30.0;
            GenerateWaveMesh(vertices, t);
            usdiMeshWriteSample(mesh, &data, t);
        }
    }


    usdiSave(ctx);
    usdiDestroyContext(ctx);
}
