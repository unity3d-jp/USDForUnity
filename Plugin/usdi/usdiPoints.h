#pragma once

namespace usdi {

class PointsSample
{
public:
};


class Points : public Xform
{
typedef Xform super;
public:
    Points(Context *ctx, Schema *parent, const UsdGeomPoints& xf);
    ~Points() override;

    UsdGeomPoints&      getUSDType() override;
    SchemaType          getType() const override;

    void                getSummary(PointsSummary& dst) const;
    bool                readSample(PointsData& dst, Time t);
    bool                writeSample(const PointsData& src, Time t);

private:
    UsdGeomPoints       m_points;
};

} // namespace usdi
