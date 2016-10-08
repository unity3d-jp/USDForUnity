#pragma once
#include "etc/Mono.h"

namespace usdi {

class nObject;
class nTransform;
class nMesh;


// native methods
extern void(nObject::*NM_Object_SetDirty)();
extern void(nTransform::*NM_Transform_SetLocalPosition)(const __m128 &pos);
extern void(nTransform::*NM_Transform_SetLocalRotation)(const __m128 &rot);
extern void(nTransform::*NM_Transform_SetLocalScale)(const __m128 &scale);
extern void(nTransform::*NM_Transform_SendTransformChanged)(int mask);
extern void(nMesh::*NM_Mesh_SetBounds)(const AABB &);


// mono classes & methods

extern MonoClass *MC_int;
extern MonoClass *MC_IntPtr;
extern MonoClass *MC_Vector2;
extern MonoClass *MC_Vector3;
extern MonoClass *MC_Quaternion;
extern MonoClass *MC_GameObject;
extern MonoClass *MC_Component;
extern MonoClass *MC_Transform;
extern MonoClass *MC_Camera;
extern MonoClass *MC_MeshFilter;
extern MonoClass *MC_MeshRenderer;
extern MonoClass *MC_Light;
extern MonoClass *MC_Mesh;
extern MonoClass *MC_usdiElement;
extern MonoClass *MC_usdiXform;
extern MonoClass *MC_usdiCamera;
extern MonoClass *MC_usdiMesh;
extern MonoClass *MC_usdiPoints;

extern MonoMethod *MM_usdiMesh_usdiAllocateChildMeshes;

extern int MF_usdiElement_m_schema;


void InitializeInternalMethods();
void ClearInternalMethodsCache();




class mObject
{
public:
    mObject(MonoObject *rep = nullptr);
    MonoObject* get() const;
    operator bool() const;

    MonoObject* mcall(MonoMethod *mm);
    MonoObject* mcall(MonoMethod *mm, void *a0);
    MonoObject* mcall(MonoMethod *mm, void *a0, void *a1);
    MonoObject* mcall(MonoMethod *mm, void *a0, void *a1, void *a2);

protected:
    MonoObject *m_rep;
};


class mGameObject : public mObject
{
typedef mObject super;
public:
    static MonoClass *MClass;

    mGameObject(MonoObject *game_object = nullptr);

    void SetActive(bool v);
    MonoObject* getComponent(MonoClass *mclass);
    MonoObject* AddComponent(MonoClass *mclass);

    template<class Component> Component getComponent() { return Component(getComponent(Component::MClass)); }
    template<class Component> Component addComponent() { return Component(addComponent(Component::MClass)); }
};


class mComponent : public mObject
{
typedef mObject super;
public:
    static MonoClass *MClass;

    mComponent(MonoObject *component = nullptr);
    mGameObject getGameObject();

protected:
    MonoObject *m_component;
};


class mTransform : public mComponent
{
typedef mComponent super;
public:
    static MonoClass *MClass;

    mTransform(MonoObject *component = nullptr);
    void setLocalPosition(const float3& v);
    void setLocalRotation(const quatf& v);
    void setLocalScale(const float3& v);
    void setParent(mTransform parent);
};


class mCamera : public mComponent
{
typedef mComponent super;
public:
    static MonoClass *MClass;

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
    static MonoClass *MClass;

    mMeshFilter(MonoObject *component = nullptr);
};


class mMeshRenderer : public mComponent
{
typedef mComponent super;
public:
    static MonoClass *MClass;

    mMeshRenderer(MonoObject *component = nullptr);
};


class mLight : public mComponent
{
    typedef mComponent super;
public:
    static MonoClass *MClass;

    mLight(MonoObject *component = nullptr);
};


class mMesh : public mObject
{
typedef mObject super;
public:
    static MonoClass *MClass;

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

} // namespace usdi
