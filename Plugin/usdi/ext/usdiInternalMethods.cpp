#include "pch.h"
#include "usdiInternal.h"
#include "usdiInternalMethods.h"
#include "usdiUnity.h"
#include "etc/Hook.h"

//#define usdiDbgForceMono


namespace usdi {

const char Sym_Object_SetDirty[] = "?SetDirty@Object@@QEAAXXZ";
const char Sym_Transform_SetLocalPosition[] = "?SetLocalPositionWithoutNotification@Transform@@QEAAXAEBU_float3@math@@@Z";
const char Sym_Transform_SetLocalRotation[] = "?SetLocalRotationWithoutNotification@Transform@@QEAAXAEBU_float4@math@@@Z";
const char Sym_Transform_SetLocalScale[] = "?SetLocalScaleWithoutNotification@Transform@@QEAAXAEBU_float3@math@@@Z";
const char Sym_Transform_SendTransformChanged[] = "?SendTransformChanged@Transform@@QEAAXH@Z";
const char Sym_Mesh_SetBounds[] = "?SetBounds@Mesh@@QEAAXAEBVAABB@@@Z";


void(uObject::*NM_Object_SetDirty)();
void(uTransform::*NM_Transform_SetLocalPosition)(const __m128 &pos);
void(uTransform::*NM_Transform_SetLocalRotation)(const __m128 &rot);
void(uTransform::*NM_Transform_SetLocalScale)(const __m128 &scale);
void(uTransform::*NM_Transform_SendTransformChanged)(int mask);
void(uMesh::*NM_Mesh_SetBounds)(const AABB &);


MonoClass *MC_Vector2;
MonoClass *MC_Vector3;
MonoClass *MC_Quaternion;
MonoClass *MC_Transform;
MonoClass *MC_Mesh;

MonoMethod *MM_Transform_set_localPosition;
MonoMethod *MM_Transform_set_localRotation;
MonoMethod *MM_Transform_set_localScale;
MonoMethod *MM_Mesh_set_vertices;
MonoMethod *MM_Mesh_set_normals;
MonoMethod *MM_Mesh_set_uv;
MonoMethod *MM_Mesh_set_bounds;
MonoMethod *MM_Mesh_SetIndices;
MonoMethod *MM_Mesh_UploadMeshData;
MonoMethod *MM_Mesh_GetNativeVertexBufferPtr;
MonoMethod *MM_Mesh_GetNativeIndexBufferPtr;



void ClearInternalMethodsCache()
{
    MM_Transform_set_localPosition = nullptr;
    MM_Transform_set_localRotation = nullptr;
    MM_Transform_set_localScale = nullptr;
    MM_Mesh_set_vertices = nullptr;
    MM_Mesh_set_normals = nullptr;
    MM_Mesh_set_uv = nullptr;
    MM_Mesh_set_bounds = nullptr;
    MM_Mesh_GetNativeVertexBufferPtr = nullptr;
    MM_Mesh_GetNativeIndexBufferPtr = nullptr;
}


void InitializeInternalMethods()
{
    static std::once_flag s_once;
    std::call_once(s_once, []() {

#ifndef usdiDbgForceMono
#define NMethod(Class, Method)  (void*&)NM_##Class##_##Method = FindSymbolByName(Sym_##Class##_##Method)

        NMethod(Object, SetDirty);
        NMethod(Transform, SetLocalPosition);
        NMethod(Transform, SetLocalRotation);
        NMethod(Transform, SetLocalScale);
        NMethod(Transform, SendTransformChanged);
        NMethod(Mesh, SetBounds);

#undef NMethod
#endif // usdiDbgForceMono


#define ICall(Name, Func, Cond) mono_add_internal_call(Name, Cond ? Func##Cpp : Func##Mono)

        ICall("UTJ.usdi::usdiUniTransformAssignXform", TransformAssignXform, NM_Transform_SetLocalPosition);
        ICall("UTJ.usdi::usdiUniTransformNotfyChange", TransformNotfyChange, NM_Transform_SendTransformChanged);
        ICall("UTJ.usdi::usdiUniMeshAssignBounds", MeshAssignBounds, NM_Mesh_SetBounds);

#undef ICall
    });


    // mono classes & methods will change when script recompiled.
    // therefore these can not be inside call_once.

    auto mdomain = mono_domain_get();
    auto masm = mono_domain_assembly_open(mdomain, "UnityEngine");
    auto mimg = mono_assembly_get_image(masm);

#define MClass(Class) MC_##Class = mono_class_from_name(mimg, "UnityEngine", #Class)
#define MMethod(Class, Method, NumArgs) MM_##Class##_##Method = mono_class_get_method_from_name(MC_##Class, #Method, NumArgs)

    MClass(Vector2);
    MClass(Vector3);
    MClass(Quaternion);

    MClass(Transform);
    MMethod(Transform, set_localPosition, 1);
    MMethod(Transform, set_localRotation, 1);
    MMethod(Transform, set_localScale, 1);

    MClass(Mesh);
    MMethod(Mesh, set_vertices, 1);
    MMethod(Mesh, set_normals, 1);
    MMethod(Mesh, set_uv, 1);
    MMethod(Mesh, set_bounds, 1);
    MMethod(Mesh, SetIndices, 3);
    MMethod(Mesh, UploadMeshData, 1);
    MMethod(Mesh, GetNativeVertexBufferPtr, 1);
    MMethod(Mesh, GetNativeIndexBufferPtr, 0);

#undef MMethod
#undef MClass
};

} // namespace usdi
