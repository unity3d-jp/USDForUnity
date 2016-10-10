#pragma once

#include "usdiExt.h"
#include "UnityEngineBinding.h"

namespace usdi {

extern void (*TransformAssignXform)(MonoObject *trans, XformData *data);
void TransformAssignXformCpp(MonoObject *trans, XformData *data);
void TransformAssignXformMono(MonoObject *trans, XformData *data);

extern void (*TransformNotfyChange)(MonoObject *trans);
void TransformNotfyChangeCpp(MonoObject *trans);
void TransformNotfyChangeMono(MonoObject *trans);

extern void (*MeshAssignBounds)(MonoObject *mesh, float3 *center, float3  *extents);
void MeshAssignBoundsCpp(MonoObject *mesh, float3 *center, float3  *extents);
void MeshAssignBoundsMono(MonoObject *mesh, float3 *center, float3  *extents);


class IUpdator;

class StreamUpdator
{
public:
    static void registerICalls();

    struct Config
    {
        bool forceSingleThread = false;
        bool directVBUpdate = true;
    };

    StreamUpdator(Context *ctx, MonoObject *component);
    ~StreamUpdator();
    void setConfig(const Config& conf);
    const Config& getConfig() const;

    void constructUnityScene();
    IUpdator* add(Schema *schema, mGameObject& go);

    void onLoad();
    void onUnload();
    void asyncUpdate(Time time);
    void update(Time time);

private:
    mTransform createNode(Schema *schema, mTransform parent);

    typedef std::unique_ptr<IUpdator> ChildPtr;
    typedef std::vector<ChildPtr> Children;

    Config m_config;
    Children m_children;
    tbb::task_group m_tasks;

    Context *m_ctx;
    mComponent m_component;
};



class IUpdator
{
public:
    IUpdator(StreamUpdator *parent, mGameObject go);
    virtual ~IUpdator();
    virtual void onLoad();
    virtual void onUnload();
    virtual void asyncUpdate(Time time);
    virtual void update(Time time);

    mGameObject& getGO() { return m_go; }

protected:
    StreamUpdator *m_parent;
    mGameObject m_go;
};


class XformUpdator : public IUpdator
{
typedef IUpdator super;
public:
    XformUpdator(StreamUpdator *parent, Xform *xf, mGameObject go);
    ~XformUpdator() override;
    void asyncUpdate(Time time) override;
    void update(Time time) override;

private:
    Xform       *m_schema;
    XformData   m_data;
protected:
    mTransform  m_mtrans;
};


class CameraUpdator : public XformUpdator
{
typedef XformUpdator super;
public:
    CameraUpdator(StreamUpdator *parent, Camera *cam, mGameObject go);
    ~CameraUpdator() override;
    void asyncUpdate(Time time) override;
    void update(Time time) override;

private:
    Camera      *m_schema;
    CameraData  m_data;
    mCamera     m_mcamera;
};


class MeshUpdator : public XformUpdator
{
typedef XformUpdator super;
public:
    union UpdateFlags
    {
        uint32_t all;
        struct {
            uint32_t points : 1;
            uint32_t normals : 1;
            uint32_t uv : 1;
            uint32_t indices : 1;
            uint32_t directVB : 1;
        };
    };

    class MeshBuffer
    {
    public:
        MeshBuffer(MeshUpdator *parent, mGameObject go);
        ~MeshBuffer();

        // async
        void kickVBUpdateTask();

        // async
        void releaseMonoArrays(UpdateFlags flags, const MeshData &data, int split);
        // async
        void copyMeshDataToMonoArrays(UpdateFlags flags, const MeshData &data);
        // async
        void copySubmeshDataToMonoArrays(UpdateFlags flags, const MeshData &data, int split);

        // sync
        void copyDataToMonoMesh(UpdateFlags flags);

    private:
        MeshUpdator *m_parent;
        mGameObject m_go;
        mMeshFilter m_mfilter;
        mMeshRenderer m_mrenderer;
        mMesh m_mmesh;

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
    typedef std::vector<SubmeshData> Splits;

    MeshUpdator(StreamUpdator *parent, Mesh *mesh, mGameObject go);
    ~MeshUpdator() override;
    void asyncUpdate(Time time) override;
    void update(Time time) override;

private:
    Mesh            *m_schema;
    MeshData        m_data, m_data_prev;
    MeshSummary     m_summary;
    Splits          m_splits;
    Buffers         m_buffers;
    UpdateFlags     m_uflags = { 0 };
    int             m_frame = 0;
};


class PointsUpdator : public XformUpdator
{
typedef XformUpdator super;
public:
    PointsUpdator(StreamUpdator *parent, Points *cam, mGameObject go);
    ~PointsUpdator() override;
    void asyncUpdate(Time time) override;
    void update(Time time) override;

private:
    Points      *m_schema;
    PointsData  m_data, m_data_prev;
};

} // namespace usdi
