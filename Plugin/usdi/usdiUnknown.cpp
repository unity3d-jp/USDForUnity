#include "pch.h"
#include "usdiInternal.h"
#include "usdiSchema.h"
#include "usdiUnknown.h"
#include "usdiContext.h"

namespace usdi {

Unknown::Unknown(Context *ctx, Schema *parent, const UsdTyped& typed)
    : super(ctx, parent, typed)
{
    usdiTrace("Unknown::Unknown(): %s\n", getPath());
}

Unknown::Unknown(Context *ctx, Schema *parent, const char *name, const char *type)
    : super(ctx, parent, name, type)
    , m_typed(m_prim)
{
    usdiTrace("Unknown::Unknown(): %s\n", getPath());
}

Unknown::~Unknown()
{
    usdiTrace("Unknown::Unknown(): %s\n", getPath());
}

UsdTyped& Unknown::getUSDSchema()
{
    return m_typed;
}


} // namespace usdi
