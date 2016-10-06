#pragma once

#include "etc/Mono.h"

namespace usdi {

void TransformAssignXformCpp(MonoObject *transform_, MonoObject *data_);
void TransformAssignXformMono(MonoObject *transform_, MonoObject *data_);
void TransformNotfyChangeCpp(MonoObject *transform_);
void TransformNotfyChangeMono(MonoObject *transform_);
void MeshAssignBoundsCpp(MonoObject *mesh_, MonoObject *center_, MonoObject  *extents_);
void MeshAssignBoundsMono(MonoObject *mesh_, MonoObject *center_, MonoObject  *extents_);


class IUpdator;

class StreamUpdator
{
public:
    StreamUpdator();
    void add(MonoObject *component);
    void asyncUpdate(Time time);
    void update(Time time);

private:
    typedef std::unique_ptr<IUpdator> ChildPtr;
    typedef std::vector<ChildPtr> Children;

    Children m_children;
};

StreamUpdator* StreamUpdator_Ctor();
void StreamUpdator_Dtor(StreamUpdator *rep);
void StreamUpdator_Add(StreamUpdator *rep, MonoObject *component);
void StreamUpdator_AsyncUpdate(StreamUpdator *rep, double time);
void StreamUpdator_Update(StreamUpdator *rep, double time);


class IUpdator
{
public:
    virtual ~IUpdator();
    virtual void asyncUpdate(Time time) = 0;
    virtual void update(Time time) = 0;
};

class XformUpdator : public IUpdator
{
public:
    XformUpdator(Xform *xf, MonoObject *component);
    ~XformUpdator() override;
    void asyncUpdate(Time time) override;
    void update(Time time) override;

protected:
    Xform       *m_xform;
    MonoObject  *m_component;
};


class MeshUpdator : public IUpdator
{
public:
    struct Segment
    {
        MonoObject *m_mmesh;
        MArray m_mvertices;
        MArray m_mnormals;
        MArray m_muv;
        MArray m_mindices;
        void *m_vb = nullptr;
        void *m_ib = nullptr;
    };
    typedef std::unique_ptr<Segment> SegmentPtr;
    typedef std::vector<SegmentPtr> Segments;

    MeshUpdator(Mesh *mesh, MonoObject *component);
    ~MeshUpdator() override;
    void asyncUpdate(Time time) override;
    void update(Time time) override;

private:
    Mesh        *m_mesh;
    MonoObject  *m_component;

    MeshData m_data, m_dataPrev;
    Segments m_segments;
};

} // namespace usdi
