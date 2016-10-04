#include "pch.h"
#include "usdiInternal.h"
#include "usdiUnity.h"
#include "etc/Mono.h"
#include "etc/Hook.h"

namespace usdi {

static const char Sym_Transform_SetLocalPosition[] = "?SetLocalPosition@Transform@@QEAAXAEBVVector3f@@@Z";
static const char Sym_Transform_SetLocalRotation[] = "?SetLocalRotation@Transform@@QEAAXAEBVQuaternionf@@@Z";
static const char Sym_Transform_SetLocalScale[] = "?SetLocalScale@Transform@@QEAAXAEBVVector3f@@@Z";
static const char Sym_Mesh_SetBounds[] = "?SetBounds@Mesh@@QEAAXAEBVAABB@@@Z";

static void(__thiscall *InternalSetLocalPosition)(void *trans, const float3 &);
static void(__thiscall *InternalSetLocalRotation)(void *trans, const quatf &);
static void(__thiscall *InternalSetLocalScale)(void *trans, const float3 &);
static void(__thiscall *InternalSetBounds)(void *mesh, const AABB &);

static MonoMethod* MonoSetLocalPosition;
static MonoMethod* MonoSetLocalRotation;
static MonoMethod* MonoSetLocalScale;
static MonoMethod* MonoSetBounds;

static struct InitializeInternalMethods
{
    InitializeInternalMethods() {
        (void*&)InternalSetLocalPosition = FindSymbolByName(Sym_Transform_SetLocalPosition);
        (void*&)InternalSetLocalRotation = FindSymbolByName(Sym_Transform_SetLocalRotation);
        (void*&)InternalSetLocalScale = FindSymbolByName(Sym_Transform_SetLocalScale);
        (void*&)InternalSetBounds = FindSymbolByName(Sym_Mesh_SetBounds);
    }
} g_InitializeUnityMethods;



template<class T> static inline T Unbox(MonoObject *mobj)
{
    return ((T**)mobj)[2];
}

template<class T> struct UnboxPointerImpl;
template<class T> struct UnboxPointerImpl<T*> { T* operator()(MonoObject *mobj) { return (T*)mobj; } };
template<class T> struct UnboxPointerImpl<T&> { T& operator()(MonoObject *mobj) { return *((T*)mobj); } };
template<class T> static inline T UnboxPointer(MonoObject *mobj) { return UnboxPointerImpl<T>()(mobj); }


static void ForceAssignXform(MonoObject *transform_, MonoObject *data_)
{
    auto* trans = Unbox<void*>(transform_);
    auto& data = UnboxPointer<XformData&>(data_);

    if (InternalSetLocalPosition) {
        if ((data.flags & (int)XformData::Flags::UpdatedPosition) != 0)
        {
            InternalSetLocalPosition(trans, data.position);
        }
        if ((data.flags & (int)XformData::Flags::UpdatedRotation) != 0)
        {
            InternalSetLocalRotation(trans, data.rotation);
        }
        if ((data.flags & (int)XformData::Flags::UpdatedScale) != 0)
        {
            InternalSetLocalScale(trans, data.scale);
        }
    }
    else {


    }
}

static void ForceAssignBounds(MonoObject *mesh_, MonoObject *center_, MonoObject  *extents_)
{
    auto* mesh = Unbox<void*>(mesh_);
    auto& center = UnboxPointer<float3&>(center_);
    auto& extents = UnboxPointer<float3&>(extents_);

    if (InternalSetBounds) {
        AABB bounds = { center, extents };
        InternalSetBounds(mesh, bounds);
    }
    else {
    }
}


void AddMonoFunctions()
{
    InitializeInternalMethods();
    mono_add_internal_call("UTJ.usdi::usdiUniForceAssignXform", ForceAssignXform);
    mono_add_internal_call("UTJ.usdi::usdiUniForceAssignBounds", ForceAssignBounds);
};

} // namespace usdi
