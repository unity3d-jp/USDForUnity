#pragma once

#include "etc/Mono.h"

namespace usdi {

void TransformAssignXformCpp(MonoObject *transform_, MonoObject *data_);
void TransformAssignXformMono(MonoObject *transform_, MonoObject *data_);
void TransformNotfyChangeCpp(MonoObject *transform_);
void TransformNotfyChangeMono(MonoObject *transform_);
void MeshAssignBoundsCpp(MonoObject *mesh_, MonoObject *center_, MonoObject  *extents_);
void MeshAssignBoundsMono(MonoObject *mesh_, MonoObject *center_, MonoObject  *extents_);


class MeshBuffer
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


    int getNumSegments() const;
    void addSegment(MonoObject *mesh);
    void update(const MeshData& md);
    Segments& getSegments();

private:
    MeshData m_data, m_dataPrev;
    Segments m_segments;
};

} // namespace usdi
