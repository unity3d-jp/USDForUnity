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
            m_children.push_back(ChildPtr(new MeshUpdator(mesh, component)));
        }
        else if (auto *cam = dynamic_cast<Camera*>(schema)) {
        }
        else if (auto *points = dynamic_cast<Points*>(schema)) {
        }
        else if (auto *xf = dynamic_cast<Xform*>(schema)) {
            m_children.push_back(ChildPtr(new XformUpdator(xf, component)));
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
    size_t grain = std::max<size_t>(m_children.size() / 32, 1);
    tbb::parallel_for(tbb::blocked_range<size_t>(0, m_children.size(), grain), [t, this](const auto& r) {
        for (size_t i = r.begin(); i != r.end(); ++i) {
            m_children[i]->asyncUpdate(t);
        }
    });
#endif
}

void StreamUpdator::update(Time time)
{
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

XformUpdator::XformUpdator(Xform *xf, MonoObject *component)
    : m_xform(xf)
    , m_component(component)
{

}

XformUpdator::~XformUpdator()
{

}

void XformUpdator::asyncUpdate(Time time)
{

}

void XformUpdator::update(Time time)
{

}


MeshUpdator::MeshUpdator(Mesh *mesh, MonoObject *component)
    : m_mesh(mesh)
    , m_component(component)
{
}

MeshUpdator::~MeshUpdator()
{
}

void MeshUpdator::asyncUpdate(Time time)
{

}

void MeshUpdator::update(Time time)
{
}


} // namespace usdi
