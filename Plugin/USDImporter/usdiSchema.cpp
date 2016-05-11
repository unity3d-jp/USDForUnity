#include "pch.h"
#include "usdiInternal.h"
#include "usdiSchema.h"

namespace usdi {

Schema::Schema(Schema *parent)
    : m_parent(parent)
{
}

Schema::~Schema()
{
}

} // namespace usdi
