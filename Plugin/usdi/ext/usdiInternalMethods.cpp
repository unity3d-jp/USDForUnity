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


void(nObject::*NM_Object_SetDirty)();
void(nTransform::*NM_Transform_SetLocalPosition)(const __m128 &pos);
void(nTransform::*NM_Transform_SetLocalRotation)(const __m128 &rot);
void(nTransform::*NM_Transform_SetLocalScale)(const __m128 &scale);
void(nTransform::*NM_Transform_SendTransformChanged)(int mask);
void(nMesh::*NM_Mesh_SetBounds)(const AABB &);


static MonoGenericInst Buf_GameObject_GetComponent_Transform;
static MonoGenericInst Buf_GameObject_GetComponent_Camera;
static MonoGenericInst Buf_GameObject_GetComponent_MeshFilter;
static MonoGenericInst Buf_GameObject_GetComponent_MeshRenderer;
static MonoGenericInst Buf_GameObject_GetComponent_Light;
static MonoGenericInst Buf_GameObject_AddComponent_Transform;
static MonoGenericInst Buf_GameObject_AddComponent_Camera;
static MonoGenericInst Buf_GameObject_AddComponent_MeshFilter;
static MonoGenericInst Buf_GameObject_AddComponent_MeshRenderer;
static MonoGenericInst Buf_GameObject_AddComponent_Light;
static MonoGenericInst Buf_Component_GetComponent_Transform;
static MonoGenericInst Buf_Component_GetComponent_Camera;

MonoClass *MC_int;
MonoClass *MC_IntPtr;
MonoClass *MC_Vector2;
MonoClass *MC_Vector3;
MonoClass *MC_Quaternion;
MonoClass *MC_GameObject;
MonoClass *MC_Component;
MonoClass *MC_Transform;
MonoClass *MC_Camera;
MonoClass *MC_MeshFilter;
MonoClass *MC_MeshRenderer;
MonoClass *MC_Light;
MonoClass *MC_Mesh;
MonoClass *MC_usdiElement;
MonoClass *MC_usdiXform;
MonoClass *MC_usdiCamera;
MonoClass *MC_usdiMesh;
MonoClass *MC_usdiPoints;

MonoMethod *MM_GameObject_SetActive;
MonoMethod *MM_GameObject_GetComponent;
MonoMethod *MM_GameObject_GetComponent_Transform;
MonoMethod *MM_GameObject_GetComponent_Camera;
MonoMethod *MM_GameObject_GetComponent_MeshFilter;
MonoMethod *MM_GameObject_GetComponent_MeshRenderer;
MonoMethod *MM_GameObject_GetComponent_Light;
MonoMethod *MM_GameObject_AddComponent;
MonoMethod *MM_GameObject_AddComponent_Transform;
MonoMethod *MM_GameObject_AddComponent_Camera;
MonoMethod *MM_GameObject_AddComponent_MeshFilter;
MonoMethod *MM_GameObject_AddComponent_MeshRenderer;
MonoMethod *MM_GameObject_AddComponent_Light;
MonoMethod *MM_Component_get_gameObject;
MonoMethod *MM_Transform_set_localPosition;
MonoMethod *MM_Transform_set_localRotation;
MonoMethod *MM_Transform_set_localScale;
MonoMethod *MM_Transform_SetParent;
MonoMethod *MM_Camera_set_nearClipPlane;
MonoMethod *MM_Camera_set_farClipPlane;
MonoMethod *MM_Camera_set_fieldOfView;
MonoMethod *MM_Camera_set_aspect;
MonoMethod *MM_MeshFilter_set_sharedMesh;
MonoMethod *MM_Light_set_color;
MonoMethod *MM_Light_set_intensity;
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


static MonoClass* FindClass(MonoImage *mimg, const char *ns, const char *classname)
{
    auto *ret = mono_class_from_name(mimg, ns, classname);
    if (!ret) {
        usdiLogWarning("FindClass: not found %s\n", classname);
        usdiDebugBreak();
    }
    return ret;
}

static MonoMethod* FindMethod(MonoClass *mclass, const char *method_name, int num_args = -1)
{
    if (!mclass) { return nullptr; }

    auto *ret = mono_class_get_method_from_name(mclass, method_name, num_args);
    if (!ret) {
        usdiLogWarning("FindMethod: not found %s\n", method_name);
        usdiDebugBreak();
    }
    return ret;
}

static MonoMethod* InstantiateGenericMethod(MonoMethod *generic_method, MonoClass *generic_param, MonoGenericInst &mgi)
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

    mgi.id = -1;
    mgi.is_open = 0;
    mgi.type_argc = 1;
    mgi.type_argv[0] = mono_class_get_type(generic_param);
    MonoGenericContext ctx = { nullptr, &mgi };
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
        ICall("UTJ.usdiStreamUpdator::_SetConfig", StreamUpdator_SetConfig);
        ICall("UTJ.usdiStreamUpdator::_Add", StreamUpdator_Add);
        ICall("UTJ.usdiStreamUpdator::_OnLoad", StreamUpdator_OnLoad);
        ICall("UTJ.usdiStreamUpdator::_OnUnload", StreamUpdator_OnUnload);
        ICall("UTJ.usdiStreamUpdator::_AsyncUpdate", StreamUpdator_AsyncUpdate);
        ICall("UTJ.usdiStreamUpdator::_Update", StreamUpdator_Update);

#undef ICall
    });


    // mono classes & methods will change when assembly is reloaded.
    // therefore these can not be inside call_once.

#define MC(Namespace, Class) MC_##Class = FindClass(mimg, Namespace, #Class)
#define MM(Class, Method, NumArgs) MM_##Class##_##Method = FindMethod(MC_##Class, #Method, NumArgs)
#define MF(Class, Name) if(MC_##Class) MF_##Class##_##Name = mono_field_get_offset(mono_class_get_field_from_name(MC_##Class, #Name))

#define MMI(Class, Method, GParam)\
    MM_##Class##_##Method##_##GParam = InstantiateGenericMethod(MM_##Class##_##Method, MC_##GParam, Buf_##Class##_##Method##_##GParam)

    auto mdomain = mono_domain_get();

    // UnityEngine methods
    auto masm = mono_domain_assembly_open(mdomain, "UnityEngine");
    if(masm) {
        auto mimg = mono_assembly_get_image(masm);

        MC_int = mono_get_int32_class();
        MC_IntPtr = mono_get_intptr_class();

        MC("UnityEngine", Vector2);
        MC("UnityEngine", Vector3);
        MC("UnityEngine", Quaternion);

        MC("UnityEngine", GameObject);
        MC("UnityEngine", Component);
        MC("UnityEngine", Transform);
        MC("UnityEngine", Camera);
        MC("UnityEngine", MeshFilter);
        MC("UnityEngine", MeshRenderer);
        MC("UnityEngine", Light);

        MC("UnityEngine", Mesh);

        MM(GameObject, SetActive, 1);
        MM(GameObject, GetComponent, 0);
        MMI(GameObject, GetComponent, Transform);
        MMI(GameObject, GetComponent, Camera);
        MMI(GameObject, GetComponent, MeshFilter);
        MMI(GameObject, GetComponent, MeshRenderer);
        MMI(GameObject, GetComponent, Light);
        MM(GameObject, AddComponent, 0);
        MMI(GameObject, AddComponent, Transform);
        MMI(GameObject, AddComponent, Camera);
        MMI(GameObject, AddComponent, MeshFilter);
        MMI(GameObject, AddComponent, MeshRenderer);
        MMI(GameObject, AddComponent, Light);

        MM(Component, get_gameObject, 0);

        MM(Transform, set_localPosition, 1);
        MM(Transform, set_localRotation, 1);
        MM(Transform, set_localScale, 1);
        MM(Transform, SetParent, 1);

        MM(Camera, set_nearClipPlane, 1);
        MM(Camera, set_farClipPlane, 1);
        MM(Camera, set_fieldOfView, 1);
        MM(Camera, set_aspect, 1);

        MM(MeshFilter, set_sharedMesh, 1);

        MM(Light, set_color, 1);
        MM(Light, set_intensity, 1);

        MM(Mesh, set_vertices, 1);
        MM(Mesh, set_normals, 1);
        MM(Mesh, set_uv, 1);
        MM(Mesh, set_bounds, 1);
        MM(Mesh, SetIndices, 3);
        MM(Mesh, UploadMeshData, 1);
        MM(Mesh, GetNativeVertexBufferPtr, 1);
        MM(Mesh, GetNativeIndexBufferPtr, 0);
    }

    // script methods
    masm = mono_domain_assembly_open(mdomain, "Assembly-CSharp");
    if(masm) {
        auto mimg = mono_assembly_get_image(masm);

        MC("UTJ", usdiElement);
        MC("UTJ", usdiXform);
        MC("UTJ", usdiCamera);
        MC("UTJ", usdiPoints);
        MC("UTJ", usdiMesh);
        MF(usdiElement, m_schema);
        MM(usdiMesh, usdiAllocateChildMeshes, 1);
    }


#undef MGethod
#undef MF
#undef MM
#undef MC
};

mObject::mObject(MonoObject *rep) : m_rep(rep) {}
MonoObject* mObject::get() const { return m_rep; }
mObject::operator bool() const { return m_rep != nullptr; }

MonoObject* mObject::mcall(MonoMethod *mm) { return MCall(m_rep, mm); }
MonoObject* mObject::mcall(MonoMethod *mm, void *a0) { return MCall(m_rep, mm, a0); }
MonoObject* mObject::mcall(MonoMethod *mm, void *a0, void *a1) { return MCall(m_rep, mm, a0, a1); }
MonoObject* mObject::mcall(MonoMethod *mm, void *a0, void *a1, void *a2) { return MCall(m_rep, mm, a0, a1, a2); }


MonoClass* mGameObject::MClass = MC_GameObject;

mGameObject::mGameObject(MonoObject *game_object)
    : super(game_object)
{
}

void mGameObject::SetActive(bool v)
{
    int a = (int)v;
    mcall(MM_GameObject_SetActive, &a);
}

MonoObject* mGameObject::getComponent(MonoClass *mclass)
{
    if (mclass == MC_Transform) { return mcall(MM_GameObject_GetComponent_Transform); }
    else if (mclass == MC_Camera) { return mcall(MM_GameObject_GetComponent_Camera); }
    else if (mclass == MC_MeshFilter) { return mcall(MM_GameObject_GetComponent_MeshFilter); }
    else if (mclass == MC_MeshRenderer) { return mcall(MM_GameObject_GetComponent_MeshRenderer); }
    else if (mclass == MC_Light) { return mcall(MM_GameObject_GetComponent_Light); }
    return nullptr;
}

MonoObject* mGameObject::AddComponent(MonoClass *mclass)
{
    if (mclass == MC_Transform) { return mcall(MM_GameObject_AddComponent_Transform); }
    else if (mclass == MC_Camera) { return mcall(MM_GameObject_AddComponent_Camera); }
    else if (mclass == MC_MeshFilter) { return mcall(MM_GameObject_AddComponent_MeshFilter); }
    else if (mclass == MC_MeshRenderer) { return mcall(MM_GameObject_AddComponent_MeshRenderer); }
    else if (mclass == MC_Light) { return mcall(MM_GameObject_AddComponent_Light); }
    return nullptr;
}


MonoClass* mComponent::MClass = MC_Component;
mComponent::mComponent(MonoObject *component) : super(component) {}
mGameObject mComponent::getGameObject()
{
    return mGameObject(mcall(MM_Component_get_gameObject));
}


MonoClass* mTransform::MClass = MC_Transform;
mTransform::mTransform(MonoObject *component) : super(component) {}
void mTransform::setLocalPosition(const float3& v) { mcall(MM_Transform_set_localPosition, (void*)&v); }
void mTransform::setLocalRotation(const quatf& v) { mcall(MM_Transform_set_localRotation, (void*)&v); }
void mTransform::setLocalScale(const float3& v) { mcall(MM_Transform_set_localScale, (void*)&v); }
void mTransform::setParent(mTransform parent) { mcall(MM_Transform_SetParent, parent.get()); }


MonoClass* mCamera::MClass = MC_Camera;
mCamera::mCamera(MonoObject *component) : super(component) {}
void mCamera::setNearClipPlane(float v) { mcall(MM_Camera_set_nearClipPlane, &v); }
void mCamera::setFarClipPlane(float v) { mcall(MM_Camera_set_farClipPlane, &v); }
void mCamera::setFieldOfView(float v) { mcall(MM_Camera_set_fieldOfView, &v); }
void mCamera::setAspect(float v) { mcall(MM_Camera_set_aspect, &v); }


MonoClass* mMeshFilter::MClass = MC_MeshFilter;
mMeshFilter::mMeshFilter(MonoObject *component) : super(component) {}


MonoClass* mMeshRenderer::MClass = MC_MeshFilter;
mMeshRenderer::mMeshRenderer(MonoObject *component) : super(component) {}


MonoClass* mLight::MClass = MC_Light;
mLight::mLight(MonoObject *component) : super(component) {}


MonoClass* mMesh::MClass = MC_Mesh;
mMesh::mMesh(MonoObject *mo) : super(mo) {}
void mMesh::setVertices(MonoArray *v) { mcall(MM_Mesh_set_vertices, v); }
void mMesh::setNormals(MonoArray *v) { mcall(MM_Mesh_set_normals, v); }
void mMesh::setUV(MonoArray *v) { mcall(MM_Mesh_set_uv, v); }
void mMesh::setIndices(MonoArray *v, int topology, int submesh) { mcall(MM_Mesh_SetIndices, &topology, &submesh); }
void mMesh::uploadMeshData(bool _fix) { int fix=_fix; mcall(MM_Mesh_UploadMeshData, &fix); }
void mMesh::setBounds(const AABB& v) { mcall(MM_Mesh_set_bounds, (void*)&v); }

bool mMesh::hasNativeBufferAPI() { return MM_Mesh_GetNativeIndexBufferPtr && MM_Mesh_GetNativeVertexBufferPtr; }
void* mMesh::getNativeVertexBufferPtr(int nth) { return Unbox<void*>(mcall(MM_Mesh_GetNativeVertexBufferPtr, &nth)); }
void* mMesh::getNativeIndexBufferPtr() { return Unbox<void*>(mcall(MM_Mesh_GetNativeIndexBufferPtr)); }

} // namespace usdi
