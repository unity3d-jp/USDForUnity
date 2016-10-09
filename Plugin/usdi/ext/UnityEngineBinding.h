#pragma once
#include "etc/Mono.h"

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


class mInt32;
class mIntPtr;
class mVector2;
class mVector3;
class mQuaternion;

class mObject;
class mGameObject;
class mComponent;
class mTransform;
class mCamera;
class mMeshFilter;
class mMeshRenderer;
class mLight;
class mMesh;


class mObject
{
public:
    mObject(MonoObject *rep = nullptr);
    MonoObject* get() const;
    operator bool() const;

    void setName(const char *name);
    std::string getName();

    MonoObject* mcall(MonoMethod *mm);
    MonoObject* mcall(MonoMethod *mm, void *a0);
    MonoObject* mcall(MonoMethod *mm, void *a0, void *a1);
    MonoObject* mcall(MonoMethod *mm, void *a0, void *a1, void *a2);

protected:
    MonoObject *m_rep;
};


class mMesh : public mObject
{
    typedef mObject super;
public:
    static mMesh New();

    mMesh(MonoObject *mo = nullptr);
    void setVertices(MonoArray *v);
    void setNormals(MonoArray *v);
    void setUV(MonoArray *v);
    void setIndices(MonoArray *v, int topology = 0, int submesh = 0);
    void uploadMeshData(bool fix);
    void setBounds(const AABB& v);

    static bool hasNativeBufferAPI();
    void* getNativeVertexBufferPtr(int nth);
    void* getNativeIndexBufferPtr();
};


class mGameObject : public mObject
{
typedef mObject super;
public:
    static mGameObject New(const char *name = "");

    mGameObject(MonoObject *game_object = nullptr);
    void SetActive(bool v);
    template<class Component> Component getComponent();
    template<class Component> Component addComponent();

    template<class Component> Component getOrAddComponent()
    {
        if (auto c = getComponent<Component>()) { return c; }
        return addComponent<Component>();
    }
};


class mComponent : public mObject
{
typedef mObject super;
public:
    mComponent(MonoObject *component = nullptr);
    mGameObject getGameObject();
};


class mTransform : public mComponent
{
typedef mComponent super;
public:
    mTransform(MonoObject *component = nullptr);
    void setLocalPosition(const float3& v);
    void setLocalRotation(const quatf& v);
    void setLocalScale(const float3& v);
    void setParent(mTransform parent);
    mTransform findChild(const char *name);
};


class mCamera : public mComponent
{
typedef mComponent super;
public:
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
    mMeshFilter(MonoObject *component = nullptr);
    mMesh getSharedMesh();
    void setSharedMesh(mMesh v);
};


class mMeshRenderer : public mComponent
{
typedef mComponent super;
public:
    mMeshRenderer(MonoObject *component = nullptr);
};


class mLight : public mComponent
{
typedef mComponent super;
public:
    mLight(MonoObject *component = nullptr);
};

} // namespace usdi
