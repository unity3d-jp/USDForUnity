#include "pch.h"
#include "usdiInternal.h"
#include "UnityEngineBinding.h"
#include "usdiComponentUpdator.h"
#include "etc/Hook.h"

//#define usdiDbgForceMono


#define mBindClass(...)\
    static mCachedClass s_class(__VA_ARGS__);\
    return s_class;

#define mBindMethod(...)\
    static mCachedMethod s_method(mTypeof<std::remove_reference<decltype(*this)>::type>(), __VA_ARGS__);
#define mBindMethodS(T, ...)\
    static mCachedMethod s_method(mTypeof<T>(), __VA_ARGS__);

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


static MonoDomain *g_mdomain;



void ClearInternalMethodsCache()
{
    mClearCache();
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

#define ICall(Name, Func) mAddMethod(Name, Func)

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

    mRebindCache();
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



#define mThisClass mTypeof<std::remove_reference<decltype(*this)>::type>()
#ifdef usdiDebug
    #define mTypeCheck() if(mobj && getClass() != mThisClass) {\
        auto *type_name = getClass().getName();\
        assert(getClass() == mThisClass);\
    } 
#else
    #define mTypeCheck()
#endif

inline mString ToMString(const char *s) { return mString::New(s); }
inline std::string ToCString(mObject v) { return mString(v.get()).toUTF8(); }

mDefImage(UnityEngine, "UnityEngine");


mDefTraits(UnityEngine, "UnityEngine", "Vector2", mVector2);
mDefTraits(UnityEngine, "UnityEngine", "Vector3", mVector3);
mDefTraits(UnityEngine, "UnityEngine", "Quaternion", mQuaternion);


mDefTraits(UnityEngine, "UnityEngine", "Object", mUObject);

mUObject::mUObject(MonoObject *rep) : super(rep) {}

void mUObject::setName(const char *name)
{
    mBindMethod("set_name");
    invoke(s_method, ToMString(name).get());
}
std::string mUObject::getName()
{
    mBindMethod("get_name");
    return ToCString(invoke(s_method));
}


mDefTraits(UnityEngine, "UnityEngine", "Mesh", mMesh);

mMesh mMesh::New()
{
    mBindMethodS(mMesh, ".ctor");
    auto ret = mObject::New<mMesh>();
    ret.invoke(s_method);
    return ret;
}

mMesh::mMesh(MonoObject *mo) : super(mo)
{
    mTypeCheck();
}
void mMesh::setVertices(MonoArray *v)
{
    mBindMethod("set_vertices", 1);
    invoke(s_method, v);
}
void mMesh::setNormals(MonoArray *v)
{
    mBindMethod("set_normals", 1);
    invoke(s_method, v);
}
void mMesh::setUV(MonoArray *v)
{
    mBindMethod("set_uv", 1);
    invoke(s_method, v);
}
void mMesh::SetTriangles(MonoArray *v)
{
    mBindMethod("SetTriangles", {"System.Int32[]", "System.Int32"});
    invoke(s_method, v);
}
void mMesh::uploadMeshData(bool _fix)
{
    mBindMethod("UploadMeshData", 1);
    int v = _fix;
    invoke(s_method, &v);
}
void mMesh::setBounds(const AABB& v)
{
    mBindMethod("set_bounds");
    invoke(s_method, (void*)&v);
}

bool mMesh::hasNativeBufferAPI()
{
    mBindMethodS(mMesh, "GetNativeVertexBufferPtr");
    return s_method;
}
void* mMesh::getNativeVertexBufferPtr(int nth)
{
    mBindMethodS(mMesh, "GetNativeVertexBufferPtr", 1);
    return invoke(s_method, &nth).unboxValue<void*>();
}
void* mMesh::getNativeIndexBufferPtr()
{
    mBindMethodS(mMesh, "GetNativeIndexBufferPtr", 0);
    return invoke(s_method).unboxValue<void*>();
}


mDefTraits(UnityEngine, "UnityEngine", "GameObject", mGameObject);

mGameObject mGameObject::New(const char *name)
{
    mBindMethodS(mGameObject, ".ctor", 1);
    auto ret = mObject::New<mGameObject>();
    ret.invoke(s_method, ToMString(name).get());
    return ret;
}

mGameObject::mGameObject(MonoObject *game_object)
    : super(game_object)
{
    mTypeCheck();
}

void mGameObject::SetActive(bool v_)
{
    mBindMethod("SetActive", 1);
    int v = (int)v_;
    invoke(s_method, &v);
}

mMethod& mGameObject::getGetComponent()
{
    mBindMethod("GetComponent", 0);
    return s_method;
}
mMethod& mGameObject::getAddComponent()
{
    mBindMethod("AddComponent", 0);
    return s_method;
}

#define Instantiate(T)\
template<> T mGameObject::getComponent()\
{\
    static mCachedIMethod s_method(getGetComponent(), mTypeof<T>());\
    return T(invoke(s_method).get());\
}\
template<> T mGameObject::addComponent()\
{\
    static mCachedIMethod s_method(getAddComponent(), mTypeof<T>());\
    return T(invoke(s_method).get());\
}
Instantiate(mTransform);
Instantiate(mCamera);
Instantiate(mMeshFilter);
Instantiate(mMeshRenderer);
Instantiate(mLight);
#undef Instantiate


mDefTraits(UnityEngine, "UnityEngine", "Component", mComponent);

mComponent::mComponent(MonoObject *component) : super(component) {}
mGameObject mComponent::getGameObject()
{
    mBindMethod("get_gameObject", 0);
    return mGameObject(invoke(s_method).get());
}


mDefTraits(UnityEngine, "UnityEngine", "Transform", mTransform);

mTransform::mTransform(MonoObject *component)
    : super(component)
{ mTypeCheck(); }
void mTransform::setLocalPosition(const float3& v)
{
    mBindMethod("set_localPosition", 1);
    invoke(s_method, (void*)&v);
}
void mTransform::setLocalRotation(const quatf& v)
{
    mBindMethod("set_localRotation", 1);
    invoke(s_method, (void*)&v);
}
void mTransform::setLocalScale(const float3& v)
{
    mBindMethod("set_localScale", 1);
    invoke(s_method, (void*)&v);
}
void mTransform::setParent(mTransform parent)
{
    mBindMethod("SetParent", 1);
    invoke(s_method, parent.get());
}
mTransform mTransform::findChild(const char *name)
{
    mBindMethod("FindChild", 1);
    return mTransform(invoke(s_method, ToMString(name).get()).get());
}


mDefTraits(UnityEngine, "UnityEngine", "Camera", mCamera);

mCamera::mCamera(MonoObject *component)
    : super(component)
{
    mTypeCheck();
}
void mCamera::setNearClipPlane(float v)
{
    mBindMethod("set_nearClipPlane", 1);
    invoke(s_method, &v);
}
void mCamera::setFarClipPlane(float v)
{
    mBindMethod("set_farClipPlane", 1);
    invoke(s_method, &v);
}
void mCamera::setFieldOfView(float v)
{
    mBindMethod("set_fieldOfView", 1);
    invoke(s_method, &v);
}
void mCamera::setAspect(float v)
{
    mBindMethod("set_aspect", 1);
    invoke(s_method, &v);
}


mDefTraits(UnityEngine, "UnityEngine", "MeshFilter", mMeshFilter);

mMeshFilter::mMeshFilter(MonoObject *component)
    : super(component)
{
    mTypeCheck();
}
mMesh mMeshFilter::getSharedMesh()
{
    mBindMethod("get_sharedMesh", 0);
    return mMesh(invoke(s_method).get());
}
void mMeshFilter::setSharedMesh(mMesh v)
{
    mBindMethod("set_sharedMesh", 1);
    invoke(s_method, v.get());
}


mDefTraits(UnityEngine, "UnityEngine", "MeshRenderer", mMeshRenderer);

mMeshRenderer::mMeshRenderer(MonoObject *component) : super(component) { mTypeCheck(); }


mDefTraits(UnityEngine, "UnityEngine", "Light", mLight);

mLight::mLight(MonoObject *component) : super(component) { mTypeCheck(); }

} // namespace usdi
