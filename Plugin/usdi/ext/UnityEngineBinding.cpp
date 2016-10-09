#include "pch.h"
#include "usdiInternal.h"
#include "UnityEngineBinding.h"
#include "usdiComponentUpdator.h"
#include "etc/Hook.h"

//#define usdiDbgForceMono


namespace usdi {

// native methods

const char Sym_Object_SetDirty[] = "?SetDirty@Object@@QEAAXXZ";
const char Sym_Transform_SetLocalPositionWithoutNotification[] = "?SetLocalPositionWithoutNotification@Transform@@QEAAXAEBU_float3@math@@@Z";
const char Sym_Transform_SetLocalRotationWithoutNotification[] = "?SetLocalRotationWithoutNotification@Transform@@QEAAXAEBU_float4@math@@@Z";
const char Sym_Transform_SetLocalScaleWithoutNotification[] = "?SetLocalScaleWithoutNotification@Transform@@QEAAXAEBU_float3@math@@@Z";
const char Sym_Transform_SendTransformChanged[] = "?SendTransformChanged@Transform@@QEAAXH@Z";
const char Sym_Mesh_SetBounds[] = "?SetBounds@Mesh@@QEAAXAEBVAABB@@@Z";

static void(nTransform::*NM_Transform_SetLocalPositionWithoutNotification)(const __m128 &pos);
static void(nTransform::*NM_Transform_SetLocalRotationWithoutNotification)(const __m128 &rot);
static void(nTransform::*NM_Transform_SetLocalScaleWithoutNotification)(const __m128 &scale);
static void(nTransform::*NM_Transform_SendTransformChanged)(int mask);
static void(nMesh::*NM_Mesh_SetBounds)(const AABB &);


// mono methods 

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

static MonoClass *MC_Int32;
static MonoClass *MC_IntPtr;
static MonoClass *MC_Vector2;
static MonoClass *MC_Vector3;
static MonoClass *MC_Quaternion;
static MonoClass *MC_Object;
static MonoClass *MC_GameObject;
static MonoClass *MC_Component;
static MonoClass *MC_Transform;
static MonoClass *MC_Camera;
static MonoClass *MC_MeshFilter;
static MonoClass *MC_MeshRenderer;
static MonoClass *MC_Light;
static MonoClass *MC_Mesh;
static MonoClass *MC_usdiElement;
static MonoClass *MC_usdiXform;
static MonoClass *MC_usdiCamera;
static MonoClass *MC_usdiMesh;
static MonoClass *MC_usdiPoints;

static MonoMethod *MM_Object_set_name;
static MonoMethod *MM_Object_get_name;
static MonoMethod *MM_GameObject_ctor;
static MonoMethod *MM_GameObject_SetActive;
static MonoMethod *MM_GameObject_GetComponent;
static MonoMethod *MM_GameObject_GetComponent_Transform;
static MonoMethod *MM_GameObject_GetComponent_Camera;
static MonoMethod *MM_GameObject_GetComponent_MeshFilter;
static MonoMethod *MM_GameObject_GetComponent_MeshRenderer;
static MonoMethod *MM_GameObject_GetComponent_Light;
static MonoMethod *MM_GameObject_AddComponent;
static MonoMethod *MM_GameObject_AddComponent_Transform;
static MonoMethod *MM_GameObject_AddComponent_Camera;
static MonoMethod *MM_GameObject_AddComponent_MeshFilter;
static MonoMethod *MM_GameObject_AddComponent_MeshRenderer;
static MonoMethod *MM_GameObject_AddComponent_Light;
static MonoMethod *MM_Component_get_gameObject;
static MonoMethod *MM_Transform_set_localPosition;
static MonoMethod *MM_Transform_set_localRotation;
static MonoMethod *MM_Transform_set_localScale;
static MonoMethod *MM_Transform_SetParent;
static MonoMethod *MM_Transform_FindChild;
static MonoMethod *MM_Camera_set_nearClipPlane;
static MonoMethod *MM_Camera_set_farClipPlane;
static MonoMethod *MM_Camera_set_fieldOfView;
static MonoMethod *MM_Camera_set_aspect;
static MonoMethod *MM_MeshFilter_get_sharedMesh;
static MonoMethod *MM_MeshFilter_set_sharedMesh;
static MonoMethod *MM_Light_set_color;
static MonoMethod *MM_Light_set_intensity;
static MonoMethod *MM_Mesh_ctor;
static MonoMethod *MM_Mesh_set_vertices;
static MonoMethod *MM_Mesh_set_normals;
static MonoMethod *MM_Mesh_set_uv;
static MonoMethod *MM_Mesh_set_bounds;
static MonoMethod *MM_Mesh_SetIndices;
static MonoMethod *MM_Mesh_UploadMeshData;
static MonoMethod *MM_Mesh_GetNativeVertexBufferPtr;
static MonoMethod *MM_Mesh_GetNativeIndexBufferPtr;


static MonoDomain *g_mdomain;

} // namespace usdi

#define Def(T) template<> MonoClass* GetMonoClass<usdi::m##T>() { return usdi::MC_##T; }
Def(Int32);
Def(IntPtr);
Def(Vector2);
Def(Vector3);
Def(Quaternion);
Def(Object);
Def(GameObject);
Def(Component);
Def(Transform);
Def(Camera);
Def(MeshFilter);
Def(MeshRenderer);
Def(Light);
Def(Mesh);
#undef Def


namespace usdi {



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

        NMethod(Transform, SetLocalPositionWithoutNotification);
        NMethod(Transform, SetLocalRotationWithoutNotification);
        NMethod(Transform, SetLocalScaleWithoutNotification);
        NMethod(Transform, SendTransformChanged);
        NMethod(Mesh, SetBounds);

#undef NMethod
#endif // usdiDbgForceMono

        TransformAssignXform = NM_Transform_SetLocalPositionWithoutNotification ? TransformAssignXformCpp : TransformAssignXformMono;
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
#define MCtor(Class, NumArgs) MM_##Class##_ctor = FindMethod(MC_##Class, ".ctor", NumArgs)

    auto mdomain = mono_domain_get();
    g_mdomain = mdomain;

    // UnityEngine methods
    auto masm = mono_domain_assembly_open(mdomain, "UnityEngine");
    if(masm) {
        auto mimg = mono_assembly_get_image(masm);

        MC_Int32 = mono_get_int32_class();
        MC_IntPtr = mono_get_intptr_class();

        MC("UnityEngine", Vector2);
        MC("UnityEngine", Vector3);
        MC("UnityEngine", Quaternion);

        MC("UnityEngine", Object);
        MC("UnityEngine", GameObject);
        MC("UnityEngine", Component);
        MC("UnityEngine", Transform);
        MC("UnityEngine", Camera);
        MC("UnityEngine", MeshFilter);
        MC("UnityEngine", MeshRenderer);
        MC("UnityEngine", Light);

        MC("UnityEngine", Mesh);

        MM(Object, set_name, 1);
        MM(Object, get_name, 0);

        MCtor(GameObject, 1);
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
        MM(Transform, FindChild, 1);

        MM(Camera, set_nearClipPlane, 1);
        MM(Camera, set_farClipPlane, 1);
        MM(Camera, set_fieldOfView, 1);
        MM(Camera, set_aspect, 1);

        MM(MeshFilter, get_sharedMesh, 0);
        MM(MeshFilter, set_sharedMesh, 1);

        MM(Light, set_color, 1);
        MM(Light, set_intensity, 1);

        MCtor(Mesh, 0);
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
    }

#undef MCtor
#undef MMI
#undef MF
#undef MM
#undef MC
};


nObject::nObject(void *rep) : m_rep(rep) {}
void* nObject::get() const { return m_rep; }
nObject::operator bool() const { return m_rep != nullptr; }


bool nTransform::isAvailable()
{
    return
        NM_Transform_SetLocalPositionWithoutNotification &&
        NM_Transform_SetLocalRotationWithoutNotification &&
        NM_Transform_SetLocalScaleWithoutNotification &&
        NM_Transform_SendTransformChanged;
}
nTransform::nTransform(void *rep) : super(rep) {}
nTransform* nTransform::self() { return (nTransform*)m_rep; }
void nTransform::setLocalPositionWithoutNotification(__m128 v) { (self()->*NM_Transform_SetLocalPositionWithoutNotification)(v); }
void nTransform::setLocalRotationWithoutNotification(__m128 v) { (self()->*NM_Transform_SetLocalRotationWithoutNotification)(v); }
void nTransform::setLocalScaleWithoutNotification(__m128 v) { (self()->*NM_Transform_SetLocalScaleWithoutNotification)(v); }
void nTransform::sendTransformChanged(int mask) { (self()->*NM_Transform_SendTransformChanged)(mask); }


bool nMesh::isAvailable()
{
    return NM_Mesh_SetBounds;
}
nMesh::nMesh(void *rep) : super(rep) {}
nMesh* nMesh::self() { return (nMesh*)m_rep; }
void nMesh::setBounds(const AABB &v) { (self()->*NM_Mesh_SetBounds)(v); }



#define mThisClass GetMonoClass<std::remove_reference<decltype(*this)>::type>()
#ifdef usdiDebug
    #define mTypeCheck() assert(!m_rep || mono_object_get_class(m_rep) == mThisClass)
#else
    #define mTypeCheck()
#endif

inline MonoString* ToMString(const char *s) { return mono_string_new(g_mdomain, s); }
inline std::string ToCString(MonoString *s) { return mono_string_to_utf8(s); }


mObject::mObject(MonoObject *rep) : m_rep(rep) {}
MonoObject* mObject::get() const { return m_rep; }
mObject::operator bool() const { return m_rep != nullptr; }

void mObject::setName(const char *name) { MCall(m_rep, MM_Object_set_name, ToMString(name)); }
std::string mObject::getName() { return ToCString((MonoString*)MCall(m_rep, MM_Object_get_name)); }

MonoObject* mObject::mcall(MonoMethod *mm) { return MCall(m_rep, mm); }
MonoObject* mObject::mcall(MonoMethod *mm, void *a0) { return MCall(m_rep, mm, a0); }
MonoObject* mObject::mcall(MonoMethod *mm, void *a0, void *a1) { return MCall(m_rep, mm, a0, a1); }
MonoObject* mObject::mcall(MonoMethod *mm, void *a0, void *a1, void *a2) { return MCall(m_rep, mm, a0, a1, a2); }


mMesh mMesh::New()
{
    MonoObject *mo = mono_object_new(g_mdomain, GetMonoClass<mMesh>());
    MCall(mo, MM_Mesh_ctor);
    return mo;
}

mMesh::mMesh(MonoObject *mo) : super(mo) { mTypeCheck(); }
void mMesh::setVertices(MonoArray *v) { mcall(MM_Mesh_set_vertices, v); }
void mMesh::setNormals(MonoArray *v) { mcall(MM_Mesh_set_normals, v); }
void mMesh::setUV(MonoArray *v) { mcall(MM_Mesh_set_uv, v); }
void mMesh::setIndices(MonoArray *v, int topology, int submesh) { mcall(MM_Mesh_SetIndices, &topology, &submesh); }
void mMesh::uploadMeshData(bool _fix) { int fix = _fix; mcall(MM_Mesh_UploadMeshData, &fix); }
void mMesh::setBounds(const AABB& v) { mcall(MM_Mesh_set_bounds, (void*)&v); }

bool mMesh::hasNativeBufferAPI() { return MM_Mesh_GetNativeIndexBufferPtr && MM_Mesh_GetNativeVertexBufferPtr; }
void* mMesh::getNativeVertexBufferPtr(int nth) { return Unbox<void*>(mcall(MM_Mesh_GetNativeVertexBufferPtr, &nth)); }
void* mMesh::getNativeIndexBufferPtr() { return Unbox<void*>(mcall(MM_Mesh_GetNativeIndexBufferPtr)); }


mGameObject mGameObject::New(const char *name)
{
    MonoObject *mo = mono_object_new(g_mdomain, GetMonoClass<mGameObject>());
    MCall(mo, MM_GameObject_ctor, ToMString(name));
    return mo;
}

mGameObject::mGameObject(MonoObject *game_object)
    : super(game_object)
{
    mTypeCheck();
}

void mGameObject::SetActive(bool v) { int a = (int)v; mcall(MM_GameObject_SetActive, &a); }
template<> mTransform    mGameObject::getComponent() { return mcall(MM_GameObject_GetComponent_Transform); }
template<> mTransform    mGameObject::addComponent() { return mcall(MM_GameObject_AddComponent_Transform); }
template<> mCamera       mGameObject::getComponent() { return mcall(MM_GameObject_GetComponent_Camera); }
template<> mCamera       mGameObject::addComponent() { return mcall(MM_GameObject_AddComponent_Camera); }
template<> mMeshFilter   mGameObject::getComponent() { return mcall(MM_GameObject_GetComponent_MeshFilter); }
template<> mMeshFilter   mGameObject::addComponent() { return mcall(MM_GameObject_AddComponent_MeshFilter); }
template<> mMeshRenderer mGameObject::getComponent() { return mcall(MM_GameObject_GetComponent_MeshRenderer); }
template<> mMeshRenderer mGameObject::addComponent() { return mcall(MM_GameObject_AddComponent_MeshRenderer); }
template<> mLight        mGameObject::getComponent() { return mcall(MM_GameObject_GetComponent_Light); }
template<> mLight        mGameObject::addComponent() { return mcall(MM_GameObject_AddComponent_Light); }


mComponent::mComponent(MonoObject *component) : super(component) {}
mGameObject mComponent::getGameObject()
{
    return mGameObject(mcall(MM_Component_get_gameObject));
}


mTransform::mTransform(MonoObject *component) : super(component) { mTypeCheck(); }
void mTransform::setLocalPosition(const float3& v) { mcall(MM_Transform_set_localPosition, (void*)&v); }
void mTransform::setLocalRotation(const quatf& v) { mcall(MM_Transform_set_localRotation, (void*)&v); }
void mTransform::setLocalScale(const float3& v) { mcall(MM_Transform_set_localScale, (void*)&v); }
void mTransform::setParent(mTransform parent) { mcall(MM_Transform_SetParent, parent.get()); }
mTransform mTransform::findChild(const char *name) { return mcall(MM_Transform_FindChild, mono_string_new(g_mdomain, name)); }


mCamera::mCamera(MonoObject *component) : super(component) { mTypeCheck(); }
void mCamera::setNearClipPlane(float v) { mcall(MM_Camera_set_nearClipPlane, &v); }
void mCamera::setFarClipPlane(float v) { mcall(MM_Camera_set_farClipPlane, &v); }
void mCamera::setFieldOfView(float v) { mcall(MM_Camera_set_fieldOfView, &v); }
void mCamera::setAspect(float v) { mcall(MM_Camera_set_aspect, &v); }


mMeshFilter::mMeshFilter(MonoObject *component) : super(component) { mTypeCheck(); }
mMesh mMeshFilter::getSharedMesh() { return mcall(MM_MeshFilter_get_sharedMesh); }
void mMeshFilter::setSharedMesh(mMesh v) { mcall(MM_MeshFilter_set_sharedMesh, v.get()); }


mMeshRenderer::mMeshRenderer(MonoObject *component) : super(component) { mTypeCheck(); }


mLight::mLight(MonoObject *component) : super(component) { mTypeCheck(); }

} // namespace usdi
