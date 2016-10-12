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
    IUpdator* add(Schema *schema, mMGameObject& go);

    void onLoad();
    void onUnload();
    void asyncUpdate(Time time);
    void update(Time time);

private:
    mMTransform createNode(Schema *schema, mMTransform& parent);

    typedef std::unique_ptr<IUpdator> ChildPtr;
    typedef std::vector<ChildPtr> Children;

    Config m_config;
    Children m_children;
    tbb::task_group m_tasks;

    Context *m_ctx;
    mMComponent m_component;
};



class IUpdator
{
public:
    IUpdator(StreamUpdator *parent, mMGameObject& go);
    virtual ~IUpdator();
    virtual void onLoad();
    virtual void onUnload();
    virtual void asyncUpdate(Time time);
    virtual void update(Time time);

    mMGameObject& getGO() { return m_go; }

protected:
    StreamUpdator *m_parent;
    mMGameObject m_go;
};


class XformUpdator : public IUpdator
{
typedef IUpdator super;
public:
    XformUpdator(StreamUpdator *parent, Xform *xf, mMGameObject& go);
    ~XformUpdator() override;
    void asyncUpdate(Time time) override;
    void update(Time time) override;

private:
    Xform       *m_schema;
    XformData   m_data;
protected:
    mMTransform  m_mtrans;
};


class CameraUpdator : public XformUpdator
{
typedef XformUpdator super;
public:
    CameraUpdator(StreamUpdator *parent, Camera *cam, mMGameObject& go);
    ~CameraUpdator() override;
    void asyncUpdate(Time time) override;
    void update(Time time) override;

private:
    Camera      *m_schema;
    CameraData  m_data;
    mMCamera     m_mcamera;
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
            uint32_t pad : 12;
            uint32_t directVB : 1;
        };

        uint32_t componentPart() { return all & 0x0000ffff; }
        uint32_t optionPart() { return all & 0xffff0000; }
    };

    class MeshBuffer
    {
    public:
        MeshBuffer(MeshUpdator *parent, mMGameObject& go, int nth);
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
        MeshUpdator *m_parent;
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

    MeshUpdator(StreamUpdator *parent, Mesh *mesh, mMGameObject& go);
    ~MeshUpdator() override;
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


class PointsUpdator : public XformUpdator
{
typedef XformUpdator super;
public:
    PointsUpdator(StreamUpdator *parent, Points *cam, mMGameObject& go);
    ~PointsUpdator() override;
    void asyncUpdate(Time time) override;
    void update(Time time) override;

private:
    Points      *m_schema;
    PointsData  m_data, m_data_prev;
};

} // namespace usdi
