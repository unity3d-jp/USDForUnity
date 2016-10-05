#include "pch.h"
#include "usdiInternal.h"
#include "usdiUnity.h"
#include "usdiInternalMethods.h"

namespace usdi {



void TransformAssignXformCpp(MonoObject *transform_, MonoObject *data_)
{
    auto& data = UnboxValue<XformData&>(data_);
    auto* trans = Unbox<uTransform>(transform_);

    if ((data.flags & (int)XformData::Flags::UpdatedPosition) != 0)
    {
        __m128 v = _mm_loadu_ps((float*)&data.position);
        (trans->*NM_Transform_SetLocalPosition)(v);
    }
    if ((data.flags & (int)XformData::Flags::UpdatedRotation) != 0)
    {
        __m128 v = _mm_loadu_ps((float*)&data.rotation);
        (trans->*NM_Transform_SetLocalRotation)(v);
    }
    if ((data.flags & (int)XformData::Flags::UpdatedScale) != 0)
    {
        __m128 v = _mm_loadu_ps((float*)&data.scale);
        (trans->*NM_Transform_SetLocalScale)(v);
    }
}

void TransformAssignXformMono(MonoObject *transform, MonoObject *data_)
{
    auto& data = UnboxValue<XformData&>(data_);

    if ((data.flags & (int)XformData::Flags::UpdatedPosition) != 0)
    {
        MCall(transform, MM_Transform_set_localPosition, &data.position);
    }
    if ((data.flags & (int)XformData::Flags::UpdatedRotation) != 0)
    {
        MCall(transform, MM_Transform_set_localRotation, &data.rotation);
    }
    if ((data.flags & (int)XformData::Flags::UpdatedScale) != 0)
    {
        MCall(transform, MM_Transform_set_localScale, &data.scale);
    }
}


void TransformNotfyChangeCpp(MonoObject *transform_)
{
    auto* trans = Unbox<uTransform>(transform_);
    (trans->*NM_Transform_SendTransformChanged)(0x1 | 0x2 | 0x8);
}

void TransformNotfyChangeMono(MonoObject *transform)
{
    // nothing to do
}


void MeshAssignBoundsCpp(MonoObject *mesh_, MonoObject *center_, MonoObject  *extents_)
{
    auto& center = UnboxValue<float3&>(center_);
    auto& extents = UnboxValue<float3&>(extents_);
    AABB bounds = { center, extents };

    auto* mesh = Unbox<uMesh>(mesh_);
    (mesh->*NM_Mesh_SetBounds)(bounds);
}

void MeshAssignBoundsMono(MonoObject *mesh, MonoObject *center_, MonoObject  *extents_)
{
    auto& center = UnboxValue<float3&>(center_);
    auto& extents = UnboxValue<float3&>(extents_);
    AABB bounds = { center, extents };

    MCall(mesh, MM_Mesh_set_bounds, &bounds);
}


} // namespace usdi
