#pragma once

namespace usdi {

struct PointsSample
{
    VtArray<GfVec3f> points;
    VtArray<GfVec3f> velocities;

    void clear();
};


class Points : public Xform
{
typedef Xform super;
friend class Context;
protected:
    Points(Context *ctx, Schema *parent, const UsdGeomPoints& xf);
    Points(Context *ctx, Schema *parent, const char *name, const char *type = "Points");
    ~Points() override;

public:
    using UsdType = UsdGeomPoints;

    UsdGeomPoints&          getUSDSchema() override;
    void                    updateSample(Time t) override;

    const PointsSummary&    getSummary() const;
    bool                    readSample(PointsData& dst, Time t, bool copy);
    bool                    writeSample(const PointsData& src, Time t);

private:
    void updateSummary() const;

private:
    UsdGeomPoints           m_points;
    PointsSample            m_sample[2], *m_front_sample = nullptr;

    mutable bool            m_summary_needs_update = true;
    mutable PointsSummary   m_summary;
};

} // namespace usdi
