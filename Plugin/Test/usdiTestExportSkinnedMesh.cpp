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
using usdi::float3x3;
using usdi::float4x4;
using usdi::Weights4;
using usdi::Weights8;

const float DegToRad = 3.1415926535897932384626433832795f / 180.0f;


void TestExportSkinnedMesh(const char *filename, int cseg, int hseg)
{
    auto *ctx = usdiCreateContext();

    usdi::ExportSettings settings;
    settings.instanceable_by_default = true;
    usdiSetExportSettings(ctx, &settings);

    usdiCreateStage(ctx, filename);
    auto *root = usdiGetRoot(ctx);

    auto *xf = usdiCreateXform(ctx, root, "SkinnedMeshRoot");
    usdiPrimSetInstanceable(xf, true);

    {
        usdi::XformData data;
        usdiXformWriteSample(xf, &data);
    }

    {
        float4x4 bindposes[5] {
            { {
                { 1.0f, 0.0f, 0.0f, 0.0f },
                { 0.0f, 1.0f, 0.0f, 0.0f },
                { 0.0f, 0.0f, 1.0f, 0.0f },
                { 0.0f, 0.0f, 0.0f, 1.0f }
            } },
            { {
                { 1.0f, 0.0f, 0.0f, 0.0f },
                { 0.0f, 1.0f, 0.0f, 0.0f },
                { 0.0f, 0.0f, 1.0f, 0.0f },
                { 0.0f,-1.0f, 0.0f, 1.0f }
            } },
            { {
                { 1.0f, 0.0f, 0.0f, 0.0f },
                { 0.0f, 1.0f, 0.0f, 0.0f },
                { 0.0f, 0.0f, 1.0f, 0.0f },
                { 0.0f,-2.0f, 0.0f, 1.0f }
            } },
            { {
                { 1.0f, 0.0f, 0.0f, 0.0f },
                { 0.0f, 1.0f, 0.0f, 0.0f },
                { 0.0f, 0.0f, 1.0f, 0.0f },
                { 0.0f,-3.0f, 0.0f, 1.0f }
            } },
            { {
                { 1.0f, 0.0f, 0.0f, 0.0f },
                { 0.0f, 1.0f, 0.0f, 0.0f },
                { 0.0f, 0.0f, 1.0f, 0.0f },
                { 0.0f,-4.0f, 0.0f, 1.0f }
            } },
        };
        const char *bones[5] {
            "/SkinnedMeshRoot/Bone0",
            "/SkinnedMeshRoot/Bone0/Bone1",
            "/SkinnedMeshRoot/Bone0/Bone1/Bone2",
            "/SkinnedMeshRoot/Bone0/Bone1/Bone2/Bone3",
            "/SkinnedMeshRoot/Bone0/Bone1/Bone2/Bone3/Bone4",
        };

        std::vector<int> counts;
        std::vector<int> indices, indices2;
        std::vector<float3> points, points2;
        std::vector<float2> uv, uv2;
        std::vector<Weights4> weights, weights2;

        auto Generate = [&](int cseg, int hseg)
        {
            const float radius = 0.2f;
            const float height = 5.0f;

            // vertices
            points.resize(cseg * hseg);
            weights.resize(points.size());
            uv.resize(points.size());

            for (int ih = 0; ih < hseg; ++ih) {
                float y = (float(ih) / float(hseg - 1)) * height;
                Weights4 w;
                w.indices[0] = std::min<int>((int)y, 4);
                w.weight[0] = 1.0f;

                for (int ic = 0; ic < cseg; ++ic) {
                    int i = cseg * ih + ic;
                    float ang = ((360.0f / cseg) * ic) * DegToRad;
                    float3 pos{ std::cos(ang) * radius, y, std::sin(ang) * radius };
                    float2 t{ float(ic) / float(cseg - 1), float(ih) / float(hseg - 1), };

                    points[i] = pos;
                    uv[i] = t;
                    weights[i] = w;
                }
            }

            // topology
            int nfaces = cseg * (hseg - 1);
            int nindices = nfaces * 4;
            counts.resize(nfaces, 4);
            indices.resize(nindices);
            for (int ih = 0; ih < hseg - 1; ++ih) {
                for (int ic = 0; ic < cseg; ++ic) {
                    auto *dst = &indices[(ih * cseg + ic) * 4];
                    dst[0] = cseg * ih + ic;
                    dst[1] = cseg * (ih + 1) + ic;
                    dst[2] = cseg * (ih + 1) + ((ic + 1) % cseg);
                    dst[3] = cseg * ih + ((ic + 1) % cseg);
                }
            }
        };

        auto Expand = [&]()
        {
            size_t n = indices.size();
            points2.resize(n);
            uv2.resize(n);
            weights2.resize(n);
            indices2.resize(n);
            for (size_t i = 0; i < n; ++i) {
                int ii = indices[i];
                points2[i]  = points[ii];
                uv2[i]      = uv[ii];
                weights2[i] = weights[ii];
                indices2[i] = i;
            }
        };

        {
            usdi::Mesh *mesh = nullptr;
            usdi::MeshData data;
            data.num_bones = 5;
            data.max_bone_weights = 4;
            data.bones = (char**)bones;
            data.root_bone = (char*)bones[0];
            data.bindposes = bindposes;


            Generate(3, hseg);
            Expand();
            data.num_counts = counts.size();
            data.num_indices = indices2.size();
            data.num_points = points2.size();
            data.counts = counts.data();
            data.indices = indices2.data();
            data.points = points2.data();
            data.uvs = uv2.data();
            data.weights4 = weights2.data();

            usdiPrimBeginEditVariant(xf, "Shape", "Triangular");
            mesh = usdiCreateMesh(ctx, xf, "Triangular");
            usdiMeshWriteSample(mesh, &data);
            usdiMeshPreComputeNormals(mesh, false);
            usdiPrimEndEditVariant(xf);


            Generate(4, hseg);
            Expand();
            data.num_counts = counts.size();
            data.num_indices = indices2.size();
            data.num_points = points2.size();
            data.counts = counts.data();
            data.indices = indices2.data();
            data.points = points2.data();
            data.uvs = uv2.data();
            data.weights4 = weights2.data();

            usdiPrimBeginEditVariant(xf, "Shape", "Square");
            mesh = usdiCreateMesh(ctx, xf, "Square");
            usdiMeshWriteSample(mesh, &data);
            usdiMeshPreComputeNormals(mesh, false);
            usdiPrimEndEditVariant(xf);


            Generate(cseg, hseg);
            data.num_counts = counts.size();
            data.num_indices = indices.size();
            data.num_points = points.size();
            data.counts = counts.data();
            data.indices = indices.data();
            data.points = points.data();
            data.uvs = uv.data();
            data.weights4 = weights.data();

            usdiPrimBeginEditVariant(xf, "Shape", "Cylinder");
            mesh = usdiCreateMesh(ctx, xf, "Cylinder");
            usdiMeshWriteSample(mesh, &data);
            usdiMeshPreComputeNormals(mesh, false);
            usdiPrimEndEditVariant(xf);
        }


        {
            auto *bone1 = usdiCreateXform(ctx, xf, "Bone0");
            auto *bone2 = usdiCreateXform(ctx, bone1, "Bone1");
            auto *bone3 = usdiCreateXform(ctx, bone2, "Bone2");
            auto *bone4 = usdiCreateXform(ctx, bone3, "Bone3");
            auto *bone5 = usdiCreateXform(ctx, bone4, "Bone4");

            usdi::XformData data;
            data.position.y = 0.0f;
            usdiXformWriteSample(bone1, &data);

            data.position.y = 1.0f;

            for (int i = 0; i < 200; ++i) {
                usdi::Time t = (1.0 / 30) * i;
                float3 axis{ 1.0f, 0.0f, 0.0f };
                float angle = 20.0f * DegToRad * std::cos(float(i*5) * DegToRad);
                data.rotation = {
                    axis.x * std::sin(angle * 0.5f),
                    axis.y * std::sin(angle * 0.5f),
                    axis.z * std::sin(angle * 0.5f),
                    std::cos(angle * 0.5f),
                };
                usdiXformWriteSample(bone2, &data, t);
                usdiXformWriteSample(bone3, &data, t);
                usdiXformWriteSample(bone4, &data, t);
                usdiXformWriteSample(bone5, &data, t);
            }
        }
    }

    {
        auto *ref1 = usdiCreateXform(ctx, root, "SkinnedMeshRef1");
        usdi::XformData data;
        data.position.x = 1.5f;
        usdiXformWriteSample(ref1, &data);

        auto *ref = usdiCreateOverride(ctx, "/SkinnedMeshRef1/Ref");
        usdiPrimAddReference(ref, nullptr, "/SkinnedMeshRoot");
    }
    {
        auto *ref2 = usdiCreateXform(ctx, root, "SkinnedMeshRef2");
        usdi::XformData data;
        data.position.x = -1.5f;
        usdiXformWriteSample(ref2, &data);

        auto *ref = usdiCreateOverride(ctx, "/SkinnedMeshRef2/Ref");
        usdiPrimAddReference(ref, nullptr, "/SkinnedMeshRoot");
    }

    usdiSave(ctx);
    usdiDestroyContext(ctx);
}
