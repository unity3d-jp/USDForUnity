#include "pch.h"
#include "usdiInternal.h"
#include "UnityEngineBinding.h"
#include "usdiComponentUpdator.h"

#include "usdiContext.h"
#include "usdiSchema.h"
#include "usdiXform.h"
#include "usdiCamera.h"
#include "usdiMesh.h"
#include "usdiPoints.h"
#include "usdiTask.h"

namespace usdi {

void(*TransformAssignXform)(MonoObject *trans, XformData *data);
void(*TransformNotfyChange)(MonoObject *trans);
void(*MeshAssignBounds)(MonoObject *mesh, float3 *center, float3  *extents);


void TransformAssignXformCpp(MonoObject *trans, XformData *data_)
{
    auto& data = *data_;
    auto t = mObject(trans).unbox<nTransform>();

    if ((data.flags & (int)XformData::Flags::UpdatedPosition) != 0)
    {
        __m128 v = _mm_loadu_ps((float*)&data.position);
        t.setLocalPositionWithoutNotification(v);
    }
    if ((data.flags & (int)XformData::Flags::UpdatedRotation) != 0)
    {
        __m128 v = _mm_loadu_ps((float*)&data.rotation);
        t.setLocalRotationWithoutNotification(v);
    }
    if ((data.flags & (int)XformData::Flags::UpdatedScale) != 0)
    {
        __m128 v = _mm_loadu_ps((float*)&data.scale);
        t.setLocalScaleWithoutNotification(v);
    }
}

void TransformAssignXformMono(MonoObject *trans, XformData *data_)
{
    auto& data = *data_;
    auto t = mTransform(trans);

    if ((data.flags & (int)XformData::Flags::UpdatedPosition) != 0)
    {
        t.setLocalPosition(data.position);
    }
    if ((data.flags & (int)XformData::Flags::UpdatedRotation) != 0)
    {
        t.setLocalRotation(data.rotation);
    }
    if ((data.flags & (int)XformData::Flags::UpdatedScale) != 0)
    {
        t.setLocalScale(data.scale);
    }
}


void TransformNotfyChangeCpp(MonoObject *trans)
{
    auto t = mObject(trans).unbox<nTransform>();
    t.sendTransformChanged(0x1 | 0x2 | 0x8);
}

void TransformNotfyChangeMono(MonoObject *trans)
{
    // nothing to do
}



void MeshAssignBoundsCpp(MonoObject *mesh, float3 *center, float3  *extents)
{
    AABB bounds = { *center, *extents };

    auto m = mObject(mesh).unbox<nMesh>();
    m.setBounds(bounds);
}

void MeshAssignBoundsMono(MonoObject *mesh, float3 *center, float3  *extents)
{
    AABB bounds = { *center, *extents };

    mMesh(mesh).setBounds(bounds);
}




StreamUpdator::StreamUpdator(Context *ctx, MonoObject *component)
    : m_ctx(ctx)
    , m_component(component)
{
}

StreamUpdator::~StreamUpdator()
{
    m_tasks.wait();
}

void StreamUpdator::setConfig(const Config& conf) { m_config = conf; }
const StreamUpdator::Config& StreamUpdator::getConfig() const { return m_config; }

mTransform StreamUpdator::createNode(Schema *schema, mTransform parent)
{
    mGameObject go;
    mTransform trans;

    bool found = false;
    if (parent) {
        if (auto child = parent.findChild(schema->getName())) {
            found = true;
            trans = child;
            go = trans.getGameObject();
        }
    }

    auto *node = add(schema, go);

    if (!found) {
        if (go) {
            trans = go.getComponent<mTransform>();
            trans.setParent(parent);
        }
        else {
            trans = parent;
        }
    }

    schema->each([this, trans](Schema *c) {
        createNode(c, trans);
    });
    return trans;
}

void StreamUpdator::constructUnityScene()
{
    if (!m_ctx || !m_component) { return; }

    auto go = m_component.getGameObject();
    auto trans = go.getComponent<mTransform>();
    createNode(m_ctx->getRootNode(), trans);
}

IUpdator* StreamUpdator::add(Schema *schema, mGameObject& go)
{
    if (!schema || schema->getUserData()==this) { return nullptr; }
    schema->setUserData(this);

    IUpdator *iu = nullptr;
    if (auto *cam = dynamic_cast<Camera*>(schema)) {
        if(!go) go = mGameObject::New(schema->getName());
        iu = new CameraUpdator(this, cam, go);
    }
    else if (auto *mesh = dynamic_cast<Mesh*>(schema)) {
        if (!go) go = mGameObject::New(schema->getName());
        iu = new MeshUpdator(this, mesh, go);
    }
    else if (auto *points = dynamic_cast<Points*>(schema)) {
        if (!go) go = mGameObject::New(schema->getName());
        iu = new PointsUpdator(this, points, go);
    }
    else if (auto *xf = dynamic_cast<Xform*>(schema)) {
        if (!go) go = mGameObject::New(schema->getName());
        iu = new XformUpdator(this, xf, go);
    }

    if (iu) {
        m_children.emplace_back(iu);
    }
    return iu;
}

void StreamUpdator::onLoad()
{
    for (auto& c : m_children) { c->onLoad(); }
}

void StreamUpdator::onUnload()
{
    m_tasks.wait();
    for (auto& c : m_children) { c->onUnload(); }
}

void StreamUpdator::asyncUpdate(Time t)
{
#define usdiDbgForceSingleThread
#ifdef usdiDbgForceSingleThread
    for (auto& s : m_children) { s->asyncUpdate(t); }
#else
    if (m_config.forceSingleThread) {
        for (auto& s : m_children) { s->asyncUpdate(t); }
    }
    else {
        m_tasks.run([this, t]() {
            mAttachThread();
            size_t grain = std::max<size_t>(m_children.size() / 32, 1);
            tbb::parallel_for(tbb::blocked_range<size_t>(0, m_children.size(), grain), [t, this](const auto& r) {
                mAttachThread();
                for (size_t i = r.begin(); i != r.end(); ++i) {
                    m_children[i]->asyncUpdate(t);
                }
            });
        });
    }
#endif
}

void StreamUpdator::update(Time time)
{
    m_tasks.wait();
    for (auto& c : m_children) {
        c->update(time);
    }
}

static StreamUpdator* StreamUpdator_Ctor(Context *ctx, MonoObject *component) { return new StreamUpdator(ctx, component); }
static void StreamUpdator_Dtor(StreamUpdator *rep) { delete rep; }
static void StreamUpdator_SetConfig(StreamUpdator *rep, StreamUpdator::Config *config) { rep->setConfig(*config); }
static void StreamUpdator_ConstructScene(StreamUpdator *rep) { rep->constructUnityScene(); }
static void StreamUpdator_Add(StreamUpdator *rep, Schema *schema, MonoObject *gameobject) { rep->add(schema, mGameObject(gameobject)); }
static void StreamUpdator_OnLoad(StreamUpdator *rep) { rep->onLoad(); }
static void StreamUpdator_OnUnload(StreamUpdator *rep) { rep->onUnload(); }
static void StreamUpdator_AsyncUpdate(StreamUpdator *rep, double time) { rep->asyncUpdate(time); }
static void StreamUpdator_Update(StreamUpdator *rep, double time) { rep->update(time); }

void StreamUpdator::registerICalls()
{
    mAddMethod("UTJ.usdiStreamUpdator::_Ctor", StreamUpdator_Ctor);
    mAddMethod("UTJ.usdiStreamUpdator::_Dtor", StreamUpdator_Dtor);
    mAddMethod("UTJ.usdiStreamUpdator::_SetConfig", StreamUpdator_SetConfig);
    mAddMethod("UTJ.usdiStreamUpdator::_ConstructScene", StreamUpdator_ConstructScene);
    mAddMethod("UTJ.usdiStreamUpdator::_Add", StreamUpdator_Add);
    mAddMethod("UTJ.usdiStreamUpdator::_OnLoad", StreamUpdator_OnLoad);
    mAddMethod("UTJ.usdiStreamUpdator::_OnUnload", StreamUpdator_OnUnload);
    mAddMethod("UTJ.usdiStreamUpdator::_AsyncUpdate", StreamUpdator_AsyncUpdate);
    mAddMethod("UTJ.usdiStreamUpdator::_Update", StreamUpdator_Update);
}





IUpdator::IUpdator(StreamUpdator *parent, mGameObject go)
    : m_parent(parent)
    , m_go(go)
{}
IUpdator::~IUpdator() {}
void IUpdator::onLoad() {}
void IUpdator::onUnload() {}
void IUpdator::asyncUpdate(Time time) {}
void IUpdator::update(Time time) {}


XformUpdator::XformUpdator(StreamUpdator *parent, Xform *xf, mGameObject go)
    : super(parent, go)
    , m_schema(xf)
{
    m_mtrans = m_go.getOrAddComponent<mTransform>();
}

XformUpdator::~XformUpdator()
{

}

void XformUpdator::asyncUpdate(Time time)
{
    m_schema->updateSample(time);
    if (m_schema->needsUpdate()) {
        m_schema->readSample(m_data, time);
    }
}

void XformUpdator::update(Time time)
{
    if (m_schema->needsUpdate() && m_mtrans) {
        TransformAssignXform(m_mtrans.get(), &m_data);
    }
}


CameraUpdator::CameraUpdator(StreamUpdator *parent, Camera *cam, mGameObject go)
    : super(parent, cam, go)
    , m_schema(cam)
{
    m_mcamera = m_go.getOrAddComponent<mCamera>();
}

CameraUpdator::~CameraUpdator()
{
}

void CameraUpdator::asyncUpdate(Time time)
{
    super::asyncUpdate(time);
    if (m_schema->needsUpdate()) {
        m_schema->readSample(m_data, time);
    }
}

void CameraUpdator::update(Time time)
{
    super::update(time);
    if (m_schema->needsUpdate() && m_mcamera) {
        m_mcamera.setNearClipPlane(m_data.near_clipping_plane);
        m_mcamera.setFarClipPlane(m_data.far_clipping_plane);
        m_mcamera.setFieldOfView(m_data.field_of_view);
        //m_ucamera.setAspect(m_data.aspect_ratio);
    }
}


static void mAssignDefaultMaterial(mMeshRenderer renderer)
{
    // only available on editor
    if (!mGetImage(UnityEditor)) { return; }

    static mClass& s_EditorGUIUtility = mCreateClassCache(mGetImage(UnityEditor), "UnityEditor", "EditorGUIUtility");
    if (!s_EditorGUIUtility) { return; }
    static mMethod& s_GetBuiltinExtraResource = mCreateMethodCache(s_EditorGUIUtility, "GetBuiltinExtraResource", 2);
    if (!s_GetBuiltinExtraResource) { return; }

    void *args[] = { mGetSystemType<mMaterial>().get(), mToMString("Default-Material.mat").get() };
    auto ret = s_GetBuiltinExtraResource.invoke(nullptr, args);
    mMaterial material(ret.get());
    renderer.setSharedMaterial(material);
}

MeshUpdator::MeshBuffer::MeshBuffer(MeshUpdator *parent, mGameObject go, int nth)
    : m_parent(parent)
    , m_go(go)
    , m_nth(nth)
{
    m_mfilter = m_go.getOrAddComponent<mMeshFilter>();
    m_mrenderer = m_go.getOrAddComponent<mMeshRenderer>();
    m_mmesh = m_mfilter.getSharedMesh();
    if (!m_mmesh) {
        m_mmesh = mMesh::New();
        m_mmesh.setName(parent->m_schema->getName());
        m_mfilter.setSharedMesh(m_mmesh);
        mAssignDefaultMaterial(m_mrenderer);
    }
    m_mmesh.markDynamic();
    m_prev_vertex_count = m_mmesh.getVertexCount();
}

MeshUpdator::MeshBuffer::~MeshBuffer()
{
    usdi::VertexCommandManager::getInstance().destroyCommand(m_hcommand);
}


void MeshUpdator::MeshBuffer::copyDataToMonoArrays()
{
    auto& data = m_parent->m_data;
    auto& flags = m_parent->m_uflags;

    if (data.submeshes) {
        // copy submesh data to mono arrays
        auto& src = m_parent->m_data.submeshes[m_nth];
        if (flags.points) {
            mResize(m_mvertices, src.num_points);
            memcpy(m_mvertices->data(), src.points, sizeof(float3)*src.num_points);
        }
        if (flags.normals) {
            mResize(m_mnormals, src.num_points);
            memcpy(m_mnormals->data(), src.normals, sizeof(float3)*src.num_points);
        }
        if (flags.uv) {
            mResize(m_muv, src.num_points);
            memcpy(m_muv->data(), src.uvs, sizeof(float2)*src.num_points);
        }
        if (flags.indices) {
            mResize(m_mindices, src.num_points);
            memcpy(m_mindices->data(), src.indices, sizeof(int)*src.num_points);
        }
    }
    else {
        // copy mesh data to mono arrays
        auto& src = m_parent->m_data;
        if (flags.points) {
            mResize(m_mvertices, src.num_points);
            memcpy(m_mvertices->data(), src.points, sizeof(float3)*src.num_points);
        }
        if (flags.normals) {
            mResize(m_mnormals, src.num_points);
            memcpy(m_mnormals->data(), src.normals, sizeof(float3)*src.num_points);
        }
        if (flags.uv) {
            mResize(m_muv, src.num_points);
            memcpy(m_muv->data(), src.uvs, sizeof(float2)*src.num_points);
        }
        if (flags.indices) {
            mResize(m_mindices, src.num_indices_triangulated);
            memcpy(m_mindices->data(), src.indices_triangulated, sizeof(int)*src.num_indices_triangulated);
        }
    }
}


void MeshUpdator::MeshBuffer::kickVBUpdateTask()
{
    if (!m_hcommand) {
        m_hcommand = usdi::VertexCommandManager::getInstance().createCommand();
    }
    usdi::VertexCommandManager::getInstance()
        .update(m_hcommand, &m_parent->m_data, m_vb, nullptr);
}

void MeshUpdator::MeshBuffer::releaseMonoArrays()
{
    m_mvertices.reset();
    m_mnormals.reset();
    m_muv.reset();
    m_mindices.reset();
}

void MeshUpdator::MeshBuffer::uploadDataToMonoMesh()
{
    auto flags = m_parent->m_uflags;

    if (flags.componentPart() == 0) { return; }

    if (flags.points) {
        m_mmesh.setVertices(m_mvertices->get());
    }
    if (flags.normals) {
        m_mmesh.setNormals(m_mnormals->get());
    }
    if (flags.uv) {
        m_mmesh.setUV(m_muv->get());
    }

    if(flags.indices) {
        m_mmesh.setTriangles(m_mindices->get());
    }

    m_mmesh.uploadMeshData(false);
    if (flags.directVB) {
        m_vb = m_mmesh.getNativeVertexBufferPtr(0);
        m_ib = m_mmesh.getNativeIndexBufferPtr();
    }
}

MeshUpdator::MeshUpdator(StreamUpdator *parent, Mesh *mesh, mGameObject go)
    : super(parent, mesh, go)
    , m_schema(mesh)
{
    m_summary = mesh->getSummary();

    m_buffers.emplace_back(new MeshBuffer(this, go, 0));

    // gather existing submeshes
    for (int i = 0; ; ++i) {
        char name[128];
        sprintf(name, "Submesh [%d]", i);
        if (auto c = m_mtrans.findChild(name)) {
            m_buffers.emplace_back(new MeshBuffer(this, c.getGameObject(), (int)m_buffers.size()));
        }
        else {
            break;
        }
    }
}

MeshUpdator::~MeshUpdator()
{
}

void MeshUpdator::asyncUpdate(Time time)
{
    super::asyncUpdate(time);
    if (m_schema->needsUpdate()) {
        m_data_prev = m_data;
        m_data.submeshes = nullptr;
        m_schema->readSample(m_data, time, false);

        m_uflags.all = 0;
        m_uflags.points = m_data.points &&  (m_frame == 0 || m_summary.topology_variance != TopologyVariance::Constant);
        m_uflags.normals = m_data.normals && (m_frame == 0 || m_summary.topology_variance != TopologyVariance::Constant);
        m_uflags.uv = m_data.uvs && (m_frame == 0 || m_summary.topology_variance != TopologyVariance::Constant);
        m_uflags.indices = m_data.num_indices_triangulated && (m_frame == 0 || m_summary.topology_variance == TopologyVariance::Heterogenous);
        m_uflags.directVB =
            m_parent->getConfig().directVBUpdate && mMesh::hasNativeBufferAPI() &&
            m_summary.topology_variance == TopologyVariance::Homogenous;
        //m_uflags.directVB = false;
        bool kick_VB_update_tasks = m_buffers.front()->m_vb != nullptr;

        if (m_data.num_splits != 0) {
            // get submesh data
            m_submeshes.resize(m_data.num_splits);
            m_data.submeshes = m_submeshes.data();
            m_schema->readSample(m_data, time, false);
        }

        if (kick_VB_update_tasks) {
            for (auto& b : m_buffers) { b->kickVBUpdateTask(); }
        }
        else {
            for (auto& b : m_buffers) { b->copyDataToMonoArrays(); }
        }
    }
}

void MeshUpdator::update(Time time)
{
    super::update(time);
    if (m_schema->needsUpdate()) {
        // create submesh objects if needed
        while (m_buffers.size() < m_data.num_splits) {
            char name[128];
            sprintf(name, "Submesh [%d]", (int)m_buffers.size() - 1);
            auto go = mGameObject::New(name);
            go.getComponent<mTransform>().setParent(m_mtrans);
            m_buffers.emplace_back(new MeshBuffer(this, go, (int)m_buffers.size()));
        }

        if (m_buffers.front()->m_vb) {
            // nothing to do here
        }
        else {
            for (auto& b : m_buffers) { b->uploadDataToMonoMesh(); }
        }
    }
    ++m_frame;
}


PointsUpdator::PointsUpdator(StreamUpdator *parent, Points *points, mGameObject go)
    : super(parent, points, go)
    , m_schema(points)
{
}

PointsUpdator::~PointsUpdator()
{
}

void PointsUpdator::asyncUpdate(Time time)
{
    super::asyncUpdate(time);
    if (m_schema->needsUpdate()) {
        m_data_prev = m_data;
        m_schema->readSample(m_data, time, false);
    }
}

void PointsUpdator::update(Time time)
{
    super::update(time);
    if (m_schema->needsUpdate()) {
        // todo
    }
}

} // namespace usdi
