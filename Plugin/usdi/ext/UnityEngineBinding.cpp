#include "pch.h"
#include "usdiInternal.h"
#include "UnityEngineBinding.h"
#include "usdiComponentUpdator.h"
#include "etc/Hook.h"
#include "etc/Mono.h"

//#define usdiDbgForceMono


#define mBindMethod(...)\
    static mMethod& mBindedMethod=mCreateMethodCache(mTypeof<std::remove_reference<decltype(*this)>::type>(), __VA_ARGS__);
#define mBindStaticMethod(T, ...)\
    static mMethod& mBindedMethod=mCreateMethodCache(mTypeof<T>(), __VA_ARGS__);
#define mBindMethodFull(...)\
    static mMethod& mBindedMethod=mCreateMethodCache(__VA_ARGS__);
#define mBindedMethod s_method

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

        TransformAssign = NM_Transform_SetLocalPositionWithoutNotification ? TransformAssignN : TransformAssignM;
        TransformNotfyChange = NM_Transform_SendTransformChanged ? TransformNotfyChangeN : TransformNotfyChangeM;
        CameraAssign = CameraAssignM;
        MeshAssignBounds = NM_Mesh_SetBounds ? MeshAssignBoundsN : MeshAssignBoundsM;

        mAddMethod("UTJ.usdi::usdiUniTransformAssign", TransformAssign);
        mAddMethod("UTJ.usdi::usdiUniTransformNotfyChange", TransformNotfyChange);
        mAddMethod("UTJ.usdi::usdiUniCameraAssign", CameraAssign);
        mAddMethod("UTJ.usdi::usdiUniMeshAssignBounds", MeshAssignBounds);
        StreamUpdator::registerICalls();
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





mDefImage(UnityEngine, "UnityEngine");
mDefImage(UnityEditor, "UnityEditor");


mDefTraits(UnityEngine, "UnityEngine", "Vector2", mVector2);
mDefTraits(UnityEngine, "UnityEngine", "Vector3", mVector3);
mDefTraits(UnityEngine, "UnityEngine", "Quaternion", mQuaternion);


mDefTraits(UnityEngine, "UnityEngine", "Object", mUObject);


mUObject::mUObject(MonoObject *rep) : super(rep) {}

bool mUObject::isNull() const { return !m_rep || !*(void**)(m_rep + 1); }

mObject mUObject::getSystemType()
{
    mBindMethod("GetType", 0);
    return invoke(mBindedMethod);
}

void mUObject::setName(const char *name)
{
    mBindMethod("set_name");
    invoke(mBindedMethod, mToMString(name).get());
}
std::string mUObject::getName()
{
    mBindMethod("get_name");
    return mToCString(invoke(mBindedMethod));
}

mMethod& mUObject::getInstantiate1()
{
    mBindStaticMethod(mUObject, "Instantiate", 1);
    return mBindedMethod;
}

#define Instantiate(T)\
    template<> T mUObject::instantiate()\
    {\
        mBindMethodFull(getInstantiate1(), {&mTypeof<T>()});\
        auto ret = sinvoke(mBindedMethod);\
        mTypeCheck(ret.getClass(), mTypeof<T>());\
        return T(ret.get());\
    }

Instantiate(mMMesh);
Instantiate(mMMaterial);
Instantiate(mMGameObject);
#undef Instantiate


mDefTraits(UnityEngine, "UnityEngine", "Material", mMaterial);
mMaterial::mMaterial(MonoObject *mo) : super(mo) {}


mDefTraits(UnityEngine, "UnityEngine", "Mesh", mMesh);

mMesh mMesh::New()
{
    mBindStaticMethod(mMesh, ".ctor");
    auto ret = mObject::New<mMesh>();
    ret.invoke(mBindedMethod);
    return ret;
}

mMesh::mMesh(MonoObject *mo) : super(mo)
{
    mTypeCheckThis();
}

int mMesh::getVertexCount()
{
    mBindMethod("get_vertexCount", 0);
    return invoke(mBindedMethod).unbox<int>();
}

void mMesh::setVertices(mTArray<mVector3> v)
{
    mBindMethod("set_vertices", 1);
    invoke(mBindedMethod, v.get());
}
void mMesh::setNormals(mTArray<mVector3> v)
{
    mBindMethod("set_normals", 1);
    invoke(mBindedMethod, v.get());
}
void mMesh::setUV(mTArray<mVector2> v)
{
    mBindMethod("set_uv", 1);
    invoke(mBindedMethod, v.get());
}
void mMesh::setTriangles(mTArray<mInt32> v)
{
    mBindMethod("SetTriangles", {"System.Int32[]", "System.Int32"});
    int zero = 0;
    invoke(mBindedMethod, v.get(), &zero);
}
void mMesh::uploadMeshData(bool _fix)
{
    mBindMethod("UploadMeshData", 1);
    int v = _fix;
    invoke(mBindedMethod, &v);
}

void mMesh::markDynamic()
{
    mBindMethod("MarkDynamic", 0);
    invoke(mBindedMethod);
}

void mMesh::setBounds(const AABB& v)
{
    mBindMethod("set_bounds");
    invoke(mBindedMethod, (void*)&v);
}

bool mMesh::hasNativeBufferAPI()
{
    mBindStaticMethod(mMesh, "GetNativeVertexBufferPtr");
    return mBindedMethod;
}
void* mMesh::getNativeVertexBufferPtr(int nth)
{
    mBindStaticMethod(mMesh, "GetNativeVertexBufferPtr", 1);
    return invoke(mBindedMethod, &nth).unbox<void*>();
}
void* mMesh::getNativeIndexBufferPtr()
{
    mBindStaticMethod(mMesh, "GetNativeIndexBufferPtr", 0);
    return invoke(mBindedMethod).unbox<void*>();
}


mDefTraits(UnityEngine, "UnityEngine", "GameObject", mGameObject);

mGameObject mGameObject::New(const char *name)
{
    mBindStaticMethod(mGameObject, ".ctor", 1);
    auto ret = mObject::New<mGameObject>();
    ret.invoke(mBindedMethod, mToMString(name).get());
    return ret;
}

mGameObject::mGameObject(MonoObject *game_object)
    : super(game_object)
{
    mTypeCheckThis();
}

void mGameObject::SetActive(bool v_)
{
    mBindMethod("SetActive", 1);
    int v = (int)v_;
    invoke(mBindedMethod, &v);
}

mMethod& mGameObject::getGetComponent()
{
    mBindMethod("GetComponent", 0);
    return mBindedMethod;
}
mMethod& mGameObject::getAddComponent()
{
    mBindMethod("AddComponent", 0);
    return mBindedMethod;
}

#define Instantiate(C)\
template<> C mGameObject::getComponent()\
{\
    mBindMethodFull(getGetComponent(), {&mTypeof<C>()});\
    auto ret = invoke(mBindedMethod);\
    mTypeCheck(ret.getClass(), mTypeof<C>());\
    return C(ret.get());\
}\
template<> C mGameObject::addComponent()\
{\
    mBindMethodFull(getAddComponent(), {&mTypeof<C>()});\
    auto ret = invoke(mBindedMethod);\
    return C(ret.get());\
}

Instantiate(mMTransform);
Instantiate(mMCamera);
Instantiate(mMMeshFilter);
Instantiate(mMMeshRenderer);
Instantiate(mMLight);

#undef Instantiate


mDefTraits(UnityEngine, "UnityEngine", "Component", mComponent);

mComponent::mComponent(MonoObject *component) : super(component) {}
mMGameObject mComponent::getGameObject()
{
    mBindMethod("get_gameObject", 0);
    return mMGameObject(invoke(mBindedMethod).get());
}


mDefTraits(UnityEngine, "UnityEngine", "Transform", mTransform);

mTransform::mTransform(MonoObject *component)
    : super(component)
{ mTypeCheckThis(); }
void mTransform::setLocalPosition(const float3& v)
{
    mBindMethod("set_localPosition", 1);
    invoke(mBindedMethod, (void*)&v);
}
void mTransform::setLocalRotation(const quatf& v)
{
    mBindMethod("set_localRotation", 1);
    invoke(mBindedMethod, (void*)&v);
}
void mTransform::setLocalScale(const float3& v)
{
    mBindMethod("set_localScale", 1);
    invoke(mBindedMethod, (void*)&v);
}
void mTransform::setParent(const mMTransform& parent)
{
    mBindMethod("SetParent", 1);
    invoke(mBindedMethod, parent->get());
}
mMTransform mTransform::findChild(const char *name)
{
    mBindMethod("FindChild", 1);
    return mMTransform(invoke(mBindedMethod, mToMString(name).get()).get());
}


mDefTraits(UnityEngine, "UnityEngine", "Camera", mCamera);

mCamera::mCamera(MonoObject *component)
    : super(component)
{
    mTypeCheckThis();
}
void mCamera::setNearClipPlane(float v)
{
    mBindMethod("set_nearClipPlane", 1);
    invoke(mBindedMethod, &v);
}
void mCamera::setFarClipPlane(float v)
{
    mBindMethod("set_farClipPlane", 1);
    invoke(mBindedMethod, &v);
}
void mCamera::setFieldOfView(float v)
{
    mBindMethod("set_fieldOfView", 1);
    invoke(mBindedMethod, &v);
}
void mCamera::setAspect(float v)
{
    mBindMethod("set_aspect", 1);
    invoke(mBindedMethod, &v);
}


mDefTraits(UnityEngine, "UnityEngine", "MeshFilter", mMeshFilter);

mMeshFilter::mMeshFilter(MonoObject *component)
    : super(component)
{
    mTypeCheckThis();
}
mMMesh mMeshFilter::getSharedMesh()
{
    mBindMethod("get_sharedMesh", 0);
    return mMMesh(invoke(mBindedMethod).get());
}
void mMeshFilter::setSharedMesh(const mMMesh& v)
{
    mBindMethod("set_sharedMesh", 1);
    invoke(mBindedMethod, v->get());
}


mDefTraits(UnityEngine, "UnityEngine", "MeshRenderer", mMeshRenderer);

mMeshRenderer::mMeshRenderer(MonoObject *component) : super(component) { mTypeCheckThis(); }

void mMeshRenderer::setSharedMaterial(const mMMaterial& m)
{
    mBindMethod("set_sharedMaterial", 1);
    invoke(mBindedMethod, m->get());
}


mDefTraits(UnityEngine, "UnityEngine", "Light", mLight);

mLight::mLight(MonoObject *component) : super(component) { mTypeCheckThis(); }

} // namespace usdi
