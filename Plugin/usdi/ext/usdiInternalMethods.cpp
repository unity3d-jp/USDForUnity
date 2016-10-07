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


static void *Buf_Component_GetComponent_Transform;
static void *Buf_Component_GetComponent_Camera;

MonoClass *MC_Vector2;
MonoClass *MC_Vector3;
MonoClass *MC_Quaternion;
MonoClass *MC_Component;
MonoClass *MC_Transform;
MonoClass *MC_Camera;
MonoClass *MC_Mesh;
MonoClass *MC_usdiElement;
MonoClass *MC_usdiXform;
MonoClass *MC_usdiCamera;
MonoClass *MC_usdiMesh;
MonoClass *MC_usdiPoints;

MonoMethod *MM_Component_GetComponent;
MonoMethod *MM_Component_GetComponent_Transform;
MonoMethod *MM_Component_GetComponent_Camera;
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
MonoMethod *MM_usdiMesh_usdiAllocateChildMeshes;

int MF_usdiElement_m_schema;



void ClearInternalMethodsCache()
{
    MC_Vector2 = nullptr;
    MC_Vector3 = nullptr;
    MC_Quaternion = nullptr;
    MC_Transform = nullptr;
    MC_Mesh = nullptr;
    MC_usdiElement = nullptr;
    MC_usdiMesh = nullptr;

    MM_Transform_set_localPosition = nullptr;
    MM_Transform_set_localRotation = nullptr;
    MM_Transform_set_localScale = nullptr;
    MM_Mesh_set_vertices = nullptr;
    MM_Mesh_set_normals = nullptr;
    MM_Mesh_set_uv = nullptr;
    MM_Mesh_set_bounds = nullptr;
    MM_Mesh_SetIndices = nullptr;
    MM_Mesh_UploadMeshData = nullptr;
    MM_Mesh_GetNativeVertexBufferPtr = nullptr;
    MM_Mesh_GetNativeIndexBufferPtr = nullptr;
    MM_usdiMesh_usdiAllocateChildMeshes = nullptr;
}


static MonoMethod* InstantiateGenericMethod(MonoMethod *generic_method, MonoClass *generic_param, void *& buf)
{
    if (!generic_method) {
        usdiLogWarning("InstantiateGenericMethod: method is null\n");
        usdiDebugBreak();
        return nullptr;
    }
    if (!generic_param) {
        usdiLogWarning("InstantiateGenericMethod: paremeter is null\n");
        usdiDebugBreak();
        return nullptr;
    }

    if (!buf) {
        buf = malloc(sizeof(MonoGenericInst));
    }

    auto *mgi = (MonoGenericInst*)buf;
    mgi->id = -1;
    mgi->is_open = 0;
    mgi->type_argc = 1;
    mgi->type_argv[0] = mono_class_get_type(generic_param);
    MonoGenericContext ctx = { nullptr, mgi };
    return mono_class_inflate_generic_method(generic_method, &ctx);
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

        TransformAssignXform = NM_Transform_SetLocalPosition ? TransformAssignXformCpp : TransformAssignXformMono;
        TransformNotfyChange = NM_Transform_SendTransformChanged ? TransformNotfyChangeCpp : TransformNotfyChangeMono;
        MeshAssignBounds = NM_Mesh_SetBounds ? MeshAssignBoundsCpp : MeshAssignBoundsMono;

#define ICall(Name, Func) mono_add_internal_call(Name, Func)

        ICall("UTJ.usdi::usdiUniTransformAssignXform", TransformAssignXform);
        ICall("UTJ.usdi::usdiUniTransformNotfyChange", TransformNotfyChange);
        ICall("UTJ.usdi::usdiUniMeshAssignBounds", MeshAssignBounds);

        ICall("UTJ.usdiStreamUpdator::_Ctor", StreamUpdator_Ctor);
        ICall("UTJ.usdiStreamUpdator::_Dtor", StreamUpdator_Dtor);
        ICall("UTJ.usdiStreamUpdator::_Add", StreamUpdator_Add);
        ICall("UTJ.usdiStreamUpdator::_AsyncUpdate", StreamUpdator_AsyncUpdate);
        ICall("UTJ.usdiStreamUpdator::_Update", StreamUpdator_Update);

#undef ICall
    });


    // mono classes & methods will change when script recompiled.
    // therefore these can not be inside call_once.

#define MClass(Namespace, Class) MC_##Class = mono_class_from_name(mimg, Namespace, #Class)
#define MMethod(Class, Method, NumArgs) if(MC_##Class) MM_##Class##_##Method = mono_class_get_method_from_name(MC_##Class, #Method, NumArgs)
#define MField(Class, Name) if(MC_##Class) MF_##Class##_##Name = mono_field_get_offset(mono_class_get_field_from_name(MC_##Class, #Name))

#define MGMethod(Class, Method, GParam)\
    MM_##Class##_##Method##_##GParam = InstantiateGenericMethod(MM_##Class##_##Method, MC_##GParam, Buf_##Class##_##Method##_##GParam)

    auto mdomain = mono_domain_get();

    // UnityEngine methods
    auto masm = mono_domain_assembly_open(mdomain, "UnityEngine");
    if(masm) {
        auto mimg = mono_assembly_get_image(masm);

        MClass("UnityEngine", Vector2);
        MClass("UnityEngine", Vector3);
        MClass("UnityEngine", Quaternion);

        MClass("UnityEngine", Component);
        MMethod(Component, GetComponent, 0);
        MClass("UnityEngine", Transform);
        MClass("UnityEngine", Camera);
        MGMethod(Component, GetComponent, Transform);
        MGMethod(Component, GetComponent, Camera);

        MClass("UnityEngine", Transform);
        MMethod(Transform, set_localPosition, 1);
        MMethod(Transform, set_localRotation, 1);
        MMethod(Transform, set_localScale, 1);

        MClass("UnityEngine", Mesh);
        MMethod(Mesh, set_vertices, 1);
        MMethod(Mesh, set_normals, 1);
        MMethod(Mesh, set_uv, 1);
        MMethod(Mesh, set_bounds, 1);
        MMethod(Mesh, SetIndices, 3);
        MMethod(Mesh, UploadMeshData, 1);
        MMethod(Mesh, GetNativeVertexBufferPtr, 1);
        MMethod(Mesh, GetNativeIndexBufferPtr, 0);
    }

    // script methods
    masm = mono_domain_assembly_open(mdomain, "Assembly-CSharp");
    if(masm) {
        auto mimg = mono_assembly_get_image(masm);

        MClass("UTJ", usdiElement);
        MField(usdiElement, m_schema);

        MClass("UTJ", usdiXform);
        MClass("UTJ", usdiCamera);
        MClass("UTJ", usdiPoints);
        MClass("UTJ", usdiMesh);
        MMethod(usdiMesh, usdiAllocateChildMeshes, 0);
    }


#undef MGethod
#undef MField
#undef MMethod
#undef MClass
};

} // namespace usdi
