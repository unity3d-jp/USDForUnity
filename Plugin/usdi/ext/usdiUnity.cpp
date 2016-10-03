#include "pch.h"
#include "usdiInternal.h"
#include "usdiUnity.h"
#include "etc/Mono.h"

namespace usdi {

static int g_dirty_flags = 0x30;
static int g_transform_access = 0x80;
static int g_local_AABB = 0xf0;


struct TRS
{
    float4 t; // unity's float3 is 16 byte...
    quatf r;
    float4 s;
};

struct TransformHierarchy
{
    int _[2];
    TRS *localTransforms;
};

struct TransformAccess
{
    TransformHierarchy *hierarchy;
    uint32_t index;
};


static inline TRS& GetTRS(void *trans)
{
    auto* ta = (TransformAccess*)((size_t)trans + g_transform_access);
    return ta->hierarchy->localTransforms[ta->index];
}

void ForceAssignTRS(void *monoobj, const XformData& data)
{
    auto* trans = ((void**)monoobj)[2];
    auto& dirty = *(int*)((size_t)trans + g_dirty_flags);
    auto& trs = GetTRS(trans);

    if ((data.flags & (int)XformData::Flags::UpdatedPosition) != 0)
    {
        (float3&)trs.t = data.position;
        ++dirty;
    }
    if ((data.flags & (int)XformData::Flags::UpdatedRotation) != 0)
    {
        trs.r = data.rotation;
        ++dirty;
    }
    if ((data.flags & (int)XformData::Flags::UpdatedScale) != 0)
    {
        (float3&)trs.s = data.scale;
        ++dirty;
    }
}

void ForceAssignBounds(void *monoobj, const float3& center, const float3& extents)
{
    auto* mesh = ((void**)monoobj)[2];
    auto& dirty = *(int*)((size_t)mesh + g_dirty_flags);
    auto* bounds = (float3*)((size_t)mesh + g_local_AABB);

    bounds[0] = center;
    bounds[1] = extents * 0.5f;
    ++dirty;
}


void AddMonoFunctions()
{
    mono_add_internal_call("UTJ.usdi::usdiUniForceAssignTRS", ForceAssignTRS);
    mono_add_internal_call("UTJ.usdi::usdiUniForceAssignBounds", ForceAssignBounds);
};

} // namespace usdi
