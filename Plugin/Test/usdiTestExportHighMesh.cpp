#include <type_traits>
#include <cstdio>
#include <cmath>
#include <vector>
#include <algorithm>
#include <future>
#include <tbb/tbb.h>
#include "../usdi/usdi.h"

using usdi::float2;
using usdi::float3;
using usdi::float4;
using usdi::quatf;

const int FrameCount = 200;

static void GenerateWaveMeshTopology(
    std::vector<int>& counts,
    std::vector<int>& indices,
    const int resolution)
{
    const int num_faces = (resolution - 1) * (resolution - 1);
    const int num_indices = num_faces * 4;

    counts.resize(num_faces);
    indices.resize(num_indices);

    for (int iy = 0; iy < resolution - 1; ++iy) {
        for (int ix = 0; ix < resolution - 1; ++ix) {
            int i = (resolution -1)*iy + ix;
            counts[i] = 4;
            indices[i * 4 + 0] = resolution*iy + ix;
            indices[i * 4 + 1] = resolution*(iy + 1) + ix;
            indices[i * 4 + 2] = resolution*(iy + 1) + (ix + 1);
            indices[i * 4 + 3] = resolution*iy + (ix + 1);
        }
    }
}

static void GenerateWaveMesh(
    std::vector<float3> &points,
    std::vector<float2> &uv,
    usdi::Time t, const int resolution)
{
    const float half_res = (float)(resolution / 2);
    int num_vertices = resolution * resolution;

    points.resize(num_vertices);
    uv.resize(num_vertices);
    for (int iy = 0; iy < resolution; ++iy) {
        for (int ix = 0; ix < resolution; ++ix) {
            int i = resolution*iy + ix;
            float2 pos = {
                float(ix) / float(resolution - 1) - 0.5f,
                float(iy) / float(resolution - 1) - 0.5f
            };
            float d = std::sqrt(pos.x*pos.x + pos.y*pos.y);

            float3& v = points[i];
            v.x = pos.x;
            v.z = pos.y;
            v.y = std::sin(d * 10.0f + t * 5.0f) * std::max<float>(1.0 - d, 0.0f);

            float2& t = uv[i];
            t.x = pos.x * 0.5 + 0.5;
            t.y = pos.y * 0.5 + 0.5;
        }
    }
}


void TestExportHighMesh(const char *filename)
{
    auto *ctx = usdiCreateContext();

    usdi::ExportSettings settings;
    settings.instanceable_by_default = true;
    usdiSetExportSettings(ctx, &settings);

    usdiCreateStage(ctx, filename);
    auto *root = usdiGetRoot(ctx);

    auto *xf = usdiCreateXform(ctx, root, "Root");
    {
        usdi::XformData data;
        usdi::Time t = 0.0;
        usdiXformWriteSample(xf, &data, t);
    }

    auto *mesh = usdiCreateMesh(ctx, xf, "WaveMesh");
    {
        std::vector<std::vector<int>> counts(FrameCount);
        std::vector<std::vector<int>> indices(FrameCount);
        std::vector<std::vector<float3>> points(FrameCount);
        std::vector<std::vector<float2>> uv(FrameCount);

        usdi::Time t = 0.0;

        tbb::parallel_for(0, FrameCount, [&counts, &indices, &points, &uv](int i) {
            usdi::Time t = 1.0 / 30.0 * i;
            int resolution = 8;
            if (i < 30)      { resolution = 8; }
            else if (i < 60) { resolution = 16; }
            else if (i < 90) { resolution = 32; }
            else if (i < 120) { resolution = 64; }
            else if (i < 150) { resolution = 128; }
            else if (i < 180) { resolution = 256; }
            GenerateWaveMeshTopology(counts[i], indices[i], resolution);
            GenerateWaveMesh(points[i], uv[i], t, resolution);
        });
        for (int i = 0; i < FrameCount; ++i) {
            auto& vertices = points[i];

            usdi::Time t = 1.0 / 30.0 * i;
            usdi::MeshData data;
            data.counts = counts[i].data();
            data.num_counts = counts[i].size();
            data.indices = indices[i].data();
            data.num_indices = indices[i].size();
            data.points = points[i].data();
            data.num_points = points[i].size();
            data.uvs = uv[i].data();
            usdiMeshWriteSample(mesh, &data, t);
        }

        usdiMeshPreComputeNormals(mesh, false);
        //usdiMeshPreComputeNormals(mesh, true);
    }


    usdiSave(ctx);
    usdiDestroyContext(ctx);
}
