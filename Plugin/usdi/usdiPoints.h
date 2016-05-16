#pragma once

namespace usdi {

struct PointsSample
{
    VtArray<GfVec3f> points;
    VtArray<GfVec3f> velocities;
};


class Points : public Xform
{
typedef Xform super;
public:
    Points(Context *ctx, Schema *parent, const UsdGeomPoints& xf);
    ~Points() override;

    UsdGeomPoints&      getUSDSchema() override;

    void                getSummary(PointsSummary& dst) const;
    bool                readSample(PointsData& dst, Time t);
    bool                writeSample(const PointsData& src, Time t);

private:
    UsdGeomPoints       m_points;
};

} // namespace usdi
