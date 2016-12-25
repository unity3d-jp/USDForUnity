#pragma once

#include <vector>
#define usdiOverrideFloat4
#include "MeshUtils//MeshUtils.h"
namespace usdi {
    using namespace mu;
}
#include "usdi/usdi.h"
using usdi::float2;
using usdi::float3;
using usdi::float4;
using usdi::quatf;
using usdi::float4x4;

void GenerateOffsets(std::vector<int>& dst, const std::vector<int>& counts);

void GenerateWaveMesh(
    std::vector<int>& counts,
    std::vector<int>& indices,
    std::vector<float3> &points,
    std::vector<float2> &uv,
    float size, float height,
    const int resolution,
    usdi::Time t);

void GenerateCylinderMesh(
    std::vector<int>& counts,
    std::vector<int>& indices,
    std::vector<float3> &points,
    std::vector<float2> &uv,
    float radius, float height,
    int cseg, int hseg);

void GenerateIcoSphereMesh(
    std::vector<int>& counts,
    std::vector<int>& indices,
    std::vector<float3> &points,
    std::vector<float2> &uv,
    float radius,
    int iteration = 3);
