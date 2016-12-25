#include <cstdio>
#include <cmath>
#include <vector>
#include <algorithm>
#include <tbb/tbb.h>
#include "Mesh.h"

void TestExportHighMesh(const char *filename, int frame_count)
{
    auto *ctx = usdiCreateContext();

    usdi::ExportSettings settings;
    settings.instanceable_by_default = true;
    usdiSetExportSettings(ctx, &settings);

    usdiCreateStage(ctx, filename);
    auto *root = usdiGetRoot(ctx);

    auto *xf = usdiCreateXform(ctx, root, "WaveMeshRoot");
    usdiPrimSetInstanceable(xf, true);
    {
        usdi::XformData data;
        usdiXformWriteSample(xf, &data);
    }

    auto *mesh = usdiCreateMesh(ctx, xf, "WaveMesh");
    {
        std::vector<std::vector<int>> counts(frame_count);
        std::vector<std::vector<int>> indices(frame_count);
        std::vector<std::vector<float3>> points(frame_count);
        std::vector<std::vector<float2>> uv(frame_count);

        usdi::Time t = 0.0;

        tbb::parallel_for(0, frame_count, [&counts, &indices, &points, &uv](int i) {
            usdi::Time t = 1.0 / 30.0 * i;
            int resolution = 8;
            if (i < 30)      { resolution = 8; }
            else if (i < 60) { resolution = 16; }
            else if (i < 90) { resolution = 32; }
            else if (i < 120) { resolution = 64; }
            else if (i < 150) { resolution = 128; }
            else if (i < 180) { resolution = 256; }
            GenerateWaveMesh(counts[i], indices[i], points[i], uv[i], 1.0f, 0.5f, resolution, t);
        });
        for (int i = 0; i < frame_count; ++i) {
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

    {
        auto *ref1 = usdiCreateXform(ctx, root, "WaveMeshRef1");
        usdi::XformData data;
        data.position.x = 1.5f;
        usdiXformWriteSample(ref1, &data);

        auto *ref = usdiCreateOverride(ctx, "/WaveMeshRef1/Ref");
        usdiPrimAddReference(ref, nullptr, "/WaveMeshRoot");
    }
    {
        auto *ref2 = usdiCreateXform(ctx, root, "WaveMeshRef2");
        usdi::XformData data;
        data.position.x = -1.5f;
        usdiXformWriteSample(ref2, &data);

        auto *ref = usdiCreateOverride(ctx, "/WaveMeshRef2/Ref");
        usdiPrimAddReference(ref, nullptr, "/WaveMeshRoot");
    }

    usdiSave(ctx);
    usdiDestroyContext(ctx);
}
