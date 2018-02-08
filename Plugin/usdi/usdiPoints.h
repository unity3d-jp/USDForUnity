#pragma once

namespace usdi {


struct PointsSample
{
    VtArray<GfVec3f> points;
    VtArray<GfVec3f> velocities;
    VtArray<float>   widths;
    VtArray<int64_t> ids64;
    VtArray<int32_t> ids32;
};


class Points : public Xform
{
typedef Xform super;
public:
    DefSchemaTraits(UsdGeomPoints, "Points");

    Points(Context *ctx, Schema *parent, const UsdPrim& prim);
    Points(Context *ctx, Schema *parent, const char *name, const char *type = _getUsdTypeName());
    ~Points() override;

    void                    updateSample(Time t) override;

    const PointsSummary&    getSummary() const;
    bool                    readSample(PointsData& dst, Time t);
    bool                    writeSample(const PointsData& src, Time t);

    using SampleCallback = std::function<void(const PointsData& data, Time t)>;
    int eachSample(const SampleCallback& cb);

private:
    UsdGeomPoints           m_points;
    PointsSample            m_sample[2], *m_front_sample = nullptr;
    Attribute               *m_attr_ids64 = nullptr;
    Attribute               *m_attr_ids32 = nullptr;

    mutable bool            m_summary_needs_update = true;
    mutable PointsSummary   m_summary;
};

} // namespace usdi
