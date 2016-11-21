#pragma once

namespace usdi {

class Xform : public Schema
{
typedef Schema super;
friend class Context;
protected:
    Xform(Context *ctx, Schema *parent, const UsdPrim& prim);
    Xform(Context *ctx, Schema *parent, const char *name, const char *type = UsdTypeName);
    ~Xform() override;

public:
    using UsdType = UsdGeomXformable;
    static const char *UsdTypeName;

    void                updateSample(Time t) override;

    const XformSummary& getSummary() const;
    bool                readSample(XformData& dst, Time t);
    bool                writeSample(const XformData& src, Time t);

private:
    typedef std::vector<UsdGeomXformOp> UsdGeomXformOps;

    UsdGeomXformable    m_xf;
    UsdGeomXformOps     m_read_ops;
    UsdGeomXformOps     m_write_ops;

    XformData            m_sample;
    mutable bool         m_summary_needs_update = true;
    mutable XformSummary m_summary;
};

} // namespace usdi
