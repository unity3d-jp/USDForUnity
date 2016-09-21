#pragma once

namespace usdi {

class Unknown : public Schema
{
typedef Schema super;
public:
    Unknown(Context *ctx, Schema *parent, const UsdTyped& typed);
    Unknown(Context *ctx, Schema *parent, const char *name, const char *type);
    ~Unknown() override;

    UsdTyped&   getUSDSchema() override;
    void        updateSample(Time t) override;

private:
    UsdTyped    m_typed;
};

} // namespace usdi
