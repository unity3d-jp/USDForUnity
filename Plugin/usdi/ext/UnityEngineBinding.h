#pragma once
#include "etc/MonoWrapper.h"

namespace usdi {

void InitializeInternalMethods();
void ClearInternalMethodsCache();


// native class bindings

class nObject;
class nTransform;
class nMesh;

class nObject
{
public:
    nObject(void *rep);
    void* get() const;
    operator bool() const;

protected:
    void *m_rep;
};


class nTransform : public nObject
{
typedef nObject super;
public:
    static bool isAvailable();
    nTransform(void *rep);
    nTransform* self();
    void setLocalPositionWithoutNotification(__m128 v);
    void setLocalRotationWithoutNotification(__m128 v);
    void setLocalScaleWithoutNotification(__m128 v);
    void sendTransformChanged(int mask);
};


class nMesh : public nObject
{
typedef nObject super;
public:
    static bool isAvailable();
    nMesh(void *rep);
    nMesh* self();
    void setBounds(const AABB &);
};



// mono class bindings


struct mVector2;
struct mVector3;
struct mQuaternion;

#define DeclUClass(Type) class m##Type; using mM##Type = mManaged<m##Type>;
DeclUClass(UObject);
DeclUClass(Material);
DeclUClass(Mesh);
DeclUClass(GameObject);
DeclUClass(Component);
DeclUClass(Transform);
DeclUClass(Camera);
DeclUClass(MeshFilter);
DeclUClass(MeshRenderer);
DeclUClass(Light);
DeclUClass(Light);
#undef DeclManeged


mDeclImage(UnityEngine);
mDeclImage(UnityEditor);

struct mVector2
{
public:
    mDeclTraits();
    float x, y;
    float& operator[](int i) { return ((float*)this)[i]; }
    const float& operator[](int i) const { return ((float*)this)[i]; }
};

struct mVector3
{
public:
    mDeclTraits();
    float x, y, z;
    float& operator[](int i) { return ((float*)this)[i]; }
    const float& operator[](int i) const { return ((float*)this)[i]; }
};

struct mQuaternion
{
public:
    mDeclTraits();
    float x, y, z, w;
    float& operator[](int i) { return ((float*)this)[i]; }
    const float& operator[](int i) const { return ((float*)this)[i]; }
};


class mUObject : public mObject
{
typedef mObject super;
public:
    mDeclTraits();

    mUObject(MonoObject *rep = nullptr);

    mObject getSystemType();
    void setName(const char *name);
    std::string getName();

    template<class T> static T instantiate();

private:
    static mMethod& getInstantiate1();
};


class mMaterial : public mUObject
{
typedef mUObject super;
public:
    mDeclTraits();
    mMaterial(MonoObject *mo = nullptr);
};


class mMesh : public mUObject
{
typedef mUObject super;
public:
    mDeclTraits();
    static mMesh New();

    mMesh(MonoObject *mo = nullptr);
    int getVertexCount();
    void setVertices(mTArray<mVector3> v);
    void setNormals(mTArray<mVector3> v);
    void setUV(mTArray<mVector2> v);
    void setTriangles(mTArray<mInt32> v);
    void uploadMeshData(bool fix);
    void markDynamic();
    void setBounds(const AABB& v);

    static bool hasNativeBufferAPI();
    void* getNativeVertexBufferPtr(int nth);
    void* getNativeIndexBufferPtr();
};


class mGameObject : public mUObject
{
typedef mUObject super;
public:
    mDeclTraits();
    static mGameObject New(const char *name = "");

    mGameObject(MonoObject *game_object = nullptr);
    void SetActive(bool v);

    // C must be managed component class
    template<class C> C getComponent();
    // C must be managed component class
    template<class C> C addComponent();

    // C must be managed component class
    template<class C> C getOrAddComponent()
    {
        if (auto c = getComponent<C>()) { return c; }
        return addComponent<C>();
    }

private:
    mMethod& getGetComponent();
    mMethod& getAddComponent();
};


class mComponent : public mUObject
{
typedef mUObject super;
public:
    mDeclTraits();
    mComponent(MonoObject *component = nullptr);
    mMGameObject getGameObject();
};


class mTransform : public mComponent
{
typedef mComponent super;
public:
    mDeclTraits();
    mTransform(MonoObject *component = nullptr);
    void setLocalPosition(const float3& v);
    void setLocalRotation(const quatf& v);
    void setLocalScale(const float3& v);
    void setParent(const mMTransform& parent);
    mMTransform findChild(const char *name);
};


class mCamera : public mComponent
{
typedef mComponent super;
public:
    mDeclTraits();
    mCamera(MonoObject *component = nullptr);
    void setNearClipPlane(float v);
    void setFarClipPlane(float v);
    void setFieldOfView(float v);
    void setAspect(float v);
};


class mMeshFilter : public mComponent
{
typedef mComponent super;
public:
    mDeclTraits();
    mMeshFilter(MonoObject *component = nullptr);
    mMMesh getSharedMesh();
    void setSharedMesh(const mMMesh& v);
};


class mMeshRenderer : public mComponent
{
typedef mComponent super;
public:
    mDeclTraits();
    mMeshRenderer(MonoObject *component = nullptr);
    void setSharedMaterial(const mMMaterial& m);
};


class mLight : public mComponent
{
typedef mComponent super;
public:
    mDeclTraits();
    mLight(MonoObject *component = nullptr);
};

} // namespace usdi
