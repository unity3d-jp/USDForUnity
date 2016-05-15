#include "pch.h"
#include "usdiInternal.h"
#include "usdiSchema.h"
#include "usdiXform.h"
#include "usdiPoints.h"

namespace usdi {


Points::Points(Context *ctx, Schema *parent, const UsdGeomPoints& points)
    : super(ctx, parent, points)
    , m_points(points)
{
    usdiTrace("Points::Points(): %s\n", getPath());
}

Points::~Points()
{
    usdiTrace("Points::~Points(): %s\n", getPath());
}

UsdGeomPoints& Points::getUSDType()
{
    return m_points;
}

SchemaType Points::getType() const
{
    return SchemaType::Points;
}

void Points::getSummary(PointsSummary& dst) const
{
    // todo
}

bool Points::readSample(PointsData& dst, Time t_)
{
    return false;
}

bool Points::writeSample(const PointsData& src, Time t_)
{
    return false;
}

} // namespace usdi
