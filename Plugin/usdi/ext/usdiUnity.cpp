#include "pch.h"
#include "usdiInternal.h"
#include "usdiUnity.h"
#include "usdiInternalMethods.h"

#include "usdiSchema.h"
#include "usdiXform.h"
#include "usdiCamera.h"
#include "usdiMesh.h"
#include "usdiPoints.h"

namespace usdi {

void(*TransformAssignXform)(MonoObject *transform_, MonoObject *data_);
void(*TransformNotfyChange)(MonoObject *transform_);
void(*MeshAssignBounds)(MonoObject *mesh_, MonoObject *center_, MonoObject  *extents_);


void TransformAssignXformCpp(MonoObject *transform_, MonoObject *data_)
{
    auto& data = UnboxValue<XformData&>(data_);
    auto* trans = Unbox<uTransform>(transform_);

    if ((data.flags & (int)XformData::Flags::UpdatedPosition) != 0)
    {
        __m128 v = _mm_loadu_ps((float*)&data.position);
        (trans->*NM_Transform_SetLocalPosition)(v);
    }
    if ((data.flags & (int)XformData::Flags::UpdatedRotation) != 0)
    {
        __m128 v = _mm_loadu_ps((float*)&data.rotation);
        (trans->*NM_Transform_SetLocalRotation)(v);
    }
    if ((data.flags & (int)XformData::Flags::UpdatedScale) != 0)
    {
        __m128 v = _mm_loadu_ps((float*)&data.scale);
        (trans->*NM_Transform_SetLocalScale)(v);
    }
}

void TransformAssignXformMono(MonoObject *transform, MonoObject *data_)
{
    auto& data = UnboxValue<XformData&>(data_);

    if ((data.flags & (int)XformData::Flags::UpdatedPosition) != 0)
    {
        MCall(transform, MM_Transform_set_localPosition, &data.position);
    }
    if ((data.flags & (int)XformData::Flags::UpdatedRotation) != 0)
    {
        MCall(transform, MM_Transform_set_localRotation, &data.rotation);
    }
    if ((data.flags & (int)XformData::Flags::UpdatedScale) != 0)
    {
        MCall(transform, MM_Transform_set_localScale, &data.scale);
    }
}


void TransformNotfyChangeCpp(MonoObject *transform_)
{
    auto* trans = Unbox<uTransform>(transform_);
    (trans->*NM_Transform_SendTransformChanged)(0x1 | 0x2 | 0x8);
}

void TransformNotfyChangeMono(MonoObject *transform)
{
    // nothing to do
}



void MeshAssignBoundsCpp(MonoObject *mesh_, MonoObject *center_, MonoObject  *extents_)
{
    auto& center = UnboxValue<float3&>(center_);
    auto& extents = UnboxValue<float3&>(extents_);
    AABB bounds = { center, extents };

    auto* mesh = Unbox<uMesh>(mesh_);
    (mesh->*NM_Mesh_SetBounds)(bounds);
}

void MeshAssignBoundsMono(MonoObject *mesh, MonoObject *center_, MonoObject  *extents_)
{
    auto& center = UnboxValue<float3&>(center_);
    auto& extents = UnboxValue<float3&>(extents_);
    AABB bounds = { center, extents };

    MCall(mesh, MM_Mesh_set_bounds, &bounds);
}


StreamUpdator::StreamUpdator()
{
}

void StreamUpdator::add(MonoObject *component)
{
    auto *schema = MField<Schema*>(component, MF_usdiElement_m_schema);
    if (schema) {
        if (auto *mesh = dynamic_cast<Mesh*>(schema)) {
            m_children.emplace_back(new MeshUpdator(mesh, component));
        }
        else if (auto *cam = dynamic_cast<Camera*>(schema)) {
            m_children.emplace_back(new CameraUpdator(cam, component));
        }
        else if (auto *xf = dynamic_cast<Xform*>(schema)) {
            m_children.emplace_back(new XformUpdator(xf, component));
        }
        else if (auto *points = dynamic_cast<Points*>(schema)) {
            m_children.emplace_back(new PointsUpdator(points, component));
        }
    }
}

void StreamUpdator::asyncUpdate(Time t)
{
#ifdef usdiDbgForceSingleThread
    for (auto& s : m_children) {
        s->asyncUpdate(t);
    }
#else
    m_tasks.run([this, t] () {
        size_t grain = std::max<size_t>(m_children.size() / 32, 1);
        tbb::parallel_for(tbb::blocked_range<size_t>(0, m_children.size(), grain), [t, this](const auto& r) {
            for (size_t i = r.begin(); i != r.end(); ++i) {
                m_children[i]->asyncUpdate(t);
            }
        });
    });
#endif
}

void StreamUpdator::update(Time time)
{
    m_tasks.wait();
    for (auto& c : m_children) {
        c->update(time);
    }
}

StreamUpdator* StreamUpdator_Ctor() { return new StreamUpdator(); }
void StreamUpdator_Dtor(StreamUpdator *rep) { delete rep; }
void StreamUpdator_Add(StreamUpdator *rep, MonoObject *component) { rep->add(component); }
void StreamUpdator_AsyncUpdate(StreamUpdator *rep, double time) { rep->asyncUpdate(time); }
void StreamUpdator_Update(StreamUpdator *rep, double time) { rep->update(time); }


IUpdator::~IUpdator() {}

void IUpdator::asyncUpdate(Time time)
{

}

XformUpdator::XformUpdator(Xform *xf, MonoObject *component)
    : m_schema(xf)
    , m_component(component)
{

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
    if (m_schema->needsUpdate()) {
        // todo
    }
}


CameraUpdator::CameraUpdator(Camera *cam, MonoObject *component)
    : super(m_schema, component)
    , m_schema(cam)
    , m_component(component)
{
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
    if (m_schema->needsUpdate()) {
        // todo
    }
}



MeshUpdator::MeshUpdator(Mesh *mesh, MonoObject *component)
    : super(m_schema, component)
    , m_schema(mesh)
    , m_component(component)
{
}

MeshUpdator::~MeshUpdator()
{
}

void MeshUpdator::asyncUpdate(Time time)
{
    super::asyncUpdate(time);
    if (m_schema->needsUpdate()) {
        m_data_prev = m_data;
        m_data.splits = nullptr;
        m_schema->readSample(m_data, time, false);

        if (m_data.num_splits == 0) {
            // no split
            m_splits.resize(1);
        }
        else {
            // split
            m_splits.resize(m_data.num_splits);
            m_data.splits = m_splits.data();
            m_schema->readSample(m_data, time, false);
        }
    }
}

void MeshUpdator::update(Time time)
{
    super::update(time);
    if (m_schema->needsUpdate()) {
        // todo
    }
}


PointsUpdator::PointsUpdator(Points *points, MonoObject *component)
    : super(m_schema, component)
    , m_schema(points)
    , m_component(component)
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
