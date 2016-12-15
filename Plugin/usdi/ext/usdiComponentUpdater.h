#pragma once

#ifdef usdiEnableMono
#include "usdiExt.h"
#include "UnityEngineBinding.h"

namespace usdi {

extern void (*TransformAssign)(MonoObject *trans, XformData *data);
void TransformAssignN(MonoObject *trans, XformData *data);
void TransformAssignM(MonoObject *trans, XformData *data);

extern void (*TransformNotfyChange)(MonoObject *trans);
void TransformNotfyChangeN(MonoObject *trans);
void TransformNotfyChangeM(MonoObject *trans);

extern void(*CameraAssign)(MonoObject *trans, CameraData *data);
void CameraAssignN(MonoObject *trans, CameraData *data);
void CameraAssignM(MonoObject *trans, CameraData *data);

extern void(*TransformNotfyChange)(MonoObject *trans);

extern void (*MeshAssignBounds)(MonoObject *mesh, float3 *center, float3  *extents);
void MeshAssignBoundsN(MonoObject *mesh, float3 *center, float3  *extents);
void MeshAssignBoundsM(MonoObject *mesh, float3 *center, float3  *extents);

#ifdef usdiEnableComponentUpdator

class IUpdater;

class StreamUpdater
{
public:
    static void registerICalls();

    struct Config
    {
        bool forceSingleThread = false;
        bool directVBUpdate = true;
    };

    StreamUpdater(Context *ctx, MonoObject *component);
    ~StreamUpdater();
    void setConfig(const Config& conf);
    const Config& getConfig() const;

    void constructUnityScene();
    IUpdater* add(Schema *schema, mMGameObject& go);

    void onLoad();
    void onUnload();
    void asyncUpdate(Time time);
    void update(Time time);

private:
    mMTransform createNode(Schema *schema, mMTransform& parent);

    typedef std::unique_ptr<IUpdater> ChildPtr;
    typedef std::vector<ChildPtr> Children;

    Config m_config;
    Children m_children;
    tbb::task_group m_tasks;

    Context *m_ctx;
    mMComponent m_component;
};



class IUpdater
{
public:
    IUpdater(StreamUpdater *parent, mMGameObject& go);
    virtual ~IUpdater();
    virtual void onLoad();
    virtual void onUnload();
    virtual void asyncUpdate(Time time);
    virtual void update(Time time);

    mMGameObject& getGO() { return m_go; }

protected:
    StreamUpdater *m_parent;
    mMGameObject m_go;
};


class XformUpdater : public IUpdater
{
typedef IUpdater super;
public:
    XformUpdater(StreamUpdater *parent, Xform *xf, mMGameObject& go);
    ~XformUpdater() override;
    void asyncUpdate(Time time) override;
    void update(Time time) override;

private:
    Xform       *m_schema;
    XformData   m_data;
protected:
    mMTransform  m_mtrans;
};


class CameraUpdater : public XformUpdater
{
typedef XformUpdater super;
public:
    CameraUpdater(StreamUpdater *parent, Camera *cam, mMGameObject& go);
    ~CameraUpdater() override;
    void asyncUpdate(Time time) override;
    void update(Time time) override;

private:
    Camera      *m_schema;
    CameraData  m_data;
    mMCamera     m_mcamera;
};


class MeshUpdater : public XformUpdater
{
typedef XformUpdater super;
public:
    union UpdateFlags
    {
        uint32_t all;
        struct {
            uint32_t points : 1;
            uint32_t normals : 1;
            uint32_t uv : 1;
            uint32_t indices : 1;
            uint32_t pad : 12;
            uint32_t directVB : 1;
        };

        uint32_t componentPart() { return all & 0x0000ffff; }
        uint32_t optionPart() { return all & 0xffff0000; }
    };

    class MeshBuffer
    {
    public:
        MeshBuffer(MeshUpdater *parent, mMGameObject& go, int nth);
        ~MeshBuffer();

        // async
        void copyDataToMonoArrays();
        // async
        void kickVBUpdateTask();
        // async
        void releaseMonoArrays();
        // sync
        void uploadDataToMonoMesh();

    public:
        MeshUpdater *m_parent;
        mMGameObject m_go;
        mMMeshFilter m_mfilter;
        mMMeshRenderer m_mrenderer;
        mMMesh m_mmesh;
        int m_nth = 0;
        int m_prev_vertex_count = 0;

        mPTArray<mVector3> m_mvertices;
        mPTArray<mVector3> m_mnormals;
        mPTArray<mVector2> m_muv;
        mPTArray<mInt32> m_mindices;
        void *m_vb = nullptr;
        void *m_ib = nullptr;
        Handle m_hcommand = 0;

    };
    typedef std::unique_ptr<MeshBuffer> BufferPtr;
    typedef std::vector<BufferPtr> Buffers;
    typedef std::vector<SubmeshData> Submeshes;

    MeshUpdater(StreamUpdater *parent, Mesh *mesh, mMGameObject& go);
    ~MeshUpdater() override;
    void asyncUpdate(Time time) override;
    void update(Time time) override;

private:
    Mesh            *m_schema;
    MeshData        m_data, m_data_prev;
    MeshSummary     m_summary;
    Submeshes       m_submeshes;
    Buffers         m_buffers;
    UpdateFlags     m_uflags = { 0 };
    int             m_frame = 0;
};


class PointsUpdater : public XformUpdater
{
typedef XformUpdater super;
public:
    PointsUpdater(StreamUpdater *parent, Points *cam, mMGameObject& go);
    ~PointsUpdater() override;
    void asyncUpdate(Time time) override;
    void update(Time time) override;

private:
    Points      *m_schema;
    PointsData  m_data, m_data_prev;
};

#endif // usdiEnableComponentUpdator
} // namespace usdi
#endif // usdiEnableMono
