#pragma once

namespace usdi {

class Xform : public Schema
{
typedef Schema super;
public:
    Xform(Context *ctx, Schema *parent, const UsdGeomXformable& xf);
    ~Xform() override;

    UsdGeomXformable&   getUSDType() override;
    SchemaType          getType() const override;

    bool                readSample(XformData& dst, Time t);
    bool                writeSample(const XformData& src, Time t);

private:
    typedef std::vector<UsdGeomXformOp> UsdGeomXformOps;

    UsdGeomXformable    m_xf;
    UsdGeomXformOps     m_read_ops;
    UsdGeomXformOps     m_write_ops;
};

} // namespace usdi
