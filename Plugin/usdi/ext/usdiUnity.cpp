#include "pch.h"
#include "usdiInternal.h"
#include "usdiUnity.h"
#include "etc/Mono.h"
#include "etc/Hook.h"

namespace usdi {

static const char Sym_Object_SetDirty[] = "?SetDirty@Object@@QEAAXXZ";
static const char Sym_Transform_SetLocalPosition[] = "?SetLocalPositionWithoutNotification@Transform@@QEAAXAEBU_float3@math@@@Z";
static const char Sym_Transform_SetLocalRotation[] = "?SetLocalRotationWithoutNotification@Transform@@QEAAXAEBU_float4@math@@@Z";
static const char Sym_Transform_SetLocalScale[] = "?SetLocalScaleWithoutNotification@Transform@@QEAAXAEBU_float3@math@@@Z";
static const char Sym_Transform_SendTransformChanged[] = "?SendTransformChanged@Transform@@QEAAXH@Z";
static const char Sym_Mesh_SetBounds[] = "?SetBounds@Mesh@@QEAAXAEBVAABB@@@Z";



class Object;
class Transform;
class Mesh;

static void(Object::*Object_SetDirty)();
static void(Transform::*Transform_SetLocalPosition)(const __m128 &pos);
static void(Transform::*Transform_SetLocalRotation)(const __m128 &rot);
static void(Transform::*Transform_SetLocalScale)(const __m128 &scale);
static void(Transform::*Transform_SendTransformChanged)(int mask);
static void(Mesh::*Mesh_SetBounds)(const AABB &);


static MonoMethod *Transform_SetLocalPositionM;
static MonoMethod *Transform_SetLocalRotationM;
static MonoMethod *Transform_SetLocalScaleM;
static MonoMethod *Mesh_SetBoundsM;


static void TransformAssignXformCpp(MonoObject *transform_, MonoObject *data_);
static void TransformAssignXformMono(MonoObject *transform_, MonoObject *data_);
static void TransformNotfyChangeCpp(MonoObject *transform_);
static void TransformNotfyChangeMono(MonoObject *transform_);
static void MeshAssignBoundsCpp(MonoObject *mesh_, MonoObject *center_, MonoObject  *extents_);
static void MeshAssignBoundsMono(MonoObject *mesh_, MonoObject *center_, MonoObject  *extents_);

void InitializeInternalMethods()
{
    static std::once_flag s_once;
    std::call_once(s_once, []() {
#define Import(Class, Method)  (void*&)Class##_##Method = FindSymbolByName(Sym_##Class##_##Method)

        Import(Object, SetDirty);
        Import(Transform, SetLocalPosition);
        Import(Transform, SetLocalRotation);
        Import(Transform, SetLocalScale);
        Import(Transform, SendTransformChanged);
        Import(Mesh, SetBounds);

#undef Import

        mono_add_internal_call("UTJ.usdi::usdiUniTransformAssignXform",
            Transform_SetLocalPosition ? TransformAssignXformCpp : TransformAssignXformMono);
        mono_add_internal_call("UTJ.usdi::usdiUniTransformNotfyChange",
            Transform_SendTransformChanged ? TransformNotfyChangeCpp : TransformNotfyChangeMono);
        mono_add_internal_call("UTJ.usdi::usdiUniMeshAssignBounds",
            Mesh_SetBounds ? MeshAssignBoundsCpp : MeshAssignBoundsMono);
    });

    auto mdomain = mono_domain_get();
    auto masm = mono_domain_assembly_open(mdomain, "UnityEngine");
    auto mimg = mono_assembly_get_image(masm);
    {
        auto mclass = mono_class_from_name(mimg, "UnityEngine", "Transform");
        Transform_SetLocalPositionM = mono_class_get_method_from_name(mclass, "set_localPosition", 1);
        Transform_SetLocalRotationM = mono_class_get_method_from_name(mclass, "set_localRotation", 1);
        Transform_SetLocalScaleM = mono_class_get_method_from_name(mclass, "set_localScale", 1);
    }
    {
        auto mclass = mono_class_from_name(mimg, "UnityEngine", "Mesh");
        Mesh_SetBoundsM = mono_class_get_method_from_name(mclass, "set_bounds", 1);
    }
};

void ClearInternalMethodsCache()
{
    Transform_SetLocalPositionM = nullptr;
    Transform_SetLocalRotationM = nullptr;
    Transform_SetLocalScaleM = nullptr;
    Mesh_SetBoundsM = nullptr;
}



template<class T> static inline T* Unbox(MonoObject *mobj)
{
    return ((T**)mobj)[2];
}

template<class T> struct UnboxPointerImpl;
template<class T> struct UnboxPointerImpl<T*> { T* operator()(MonoObject *mobj) { return (T*)mobj; } };
template<class T> struct UnboxPointerImpl<T&> { T& operator()(MonoObject *mobj) { return *((T*)mobj); } };
template<class T> static inline T UnboxPointer(MonoObject *mobj) { return UnboxPointerImpl<T>()(mobj); }


static void TransformAssignXformCpp(MonoObject *transform_, MonoObject *data_)
{
    auto& data = UnboxPointer<XformData&>(data_);
    auto* trans = Unbox<Transform>(transform_);

    if ((data.flags & (int)XformData::Flags::UpdatedPosition) != 0)
    {
        __m128 v = _mm_loadu_ps((float*)&data.position);
        (trans->*Transform_SetLocalPosition)(v);
    }
    if ((data.flags & (int)XformData::Flags::UpdatedRotation) != 0)
    {
        __m128 v = _mm_loadu_ps((float*)&data.rotation);
        (trans->*Transform_SetLocalRotation)(v);
    }
    if ((data.flags & (int)XformData::Flags::UpdatedScale) != 0)
    {
        __m128 v = _mm_loadu_ps((float*)&data.scale);
        (trans->*Transform_SetLocalScale)(v);
    }
}

static void TransformAssignXformMono(MonoObject *transform, MonoObject *data_)
{
    auto& data = UnboxPointer<XformData&>(data_);

    if ((data.flags & (int)XformData::Flags::UpdatedPosition) != 0)
    {
        MonoObject *args[] = { &data.position };
        mono_runtime_invoke(Transform_SetLocalPositionM, transform, args, nullptr);
    }
    if ((data.flags & (int)XformData::Flags::UpdatedRotation) != 0)
    {
        MonoObject *args[] = { &data.rotation };
        mono_runtime_invoke(Transform_SetLocalRotationM, transform, args, nullptr);
    }
    if ((data.flags & (int)XformData::Flags::UpdatedScale) != 0)
    {
        MonoObject *args[] = { &data.scale };
        mono_runtime_invoke(Transform_SetLocalScaleM, transform, args, nullptr);
    }
}


static void TransformNotfyChangeCpp(MonoObject *transform_)
{
    auto* trans = Unbox<Transform>(transform_);
    (trans->*Transform_SendTransformChanged)(0x1 | 0x2 | 0x8);
}

static void TransformNotfyChangeMono(MonoObject *transform)
{
    // nothing to do
}


static void MeshAssignBoundsCpp(MonoObject *mesh_, MonoObject *center_, MonoObject  *extents_)
{
    auto& center = UnboxPointer<float3&>(center_);
    auto& extents = UnboxPointer<float3&>(extents_);
    AABB bounds = { center, extents };

    auto* mesh = Unbox<Mesh>(mesh_);
    (mesh->*Mesh_SetBounds)(bounds);
}

static void MeshAssignBoundsMono(MonoObject *mesh, MonoObject *center_, MonoObject  *extents_)
{
    auto& center = UnboxPointer<float3&>(center_);
    auto& extents = UnboxPointer<float3&>(extents_);
    AABB bounds = { center, extents };

    MonoObject *args[] = { &bounds };
    mono_runtime_invoke(Mesh_SetBoundsM, mesh, args, nullptr);
}


} // namespace usdi
