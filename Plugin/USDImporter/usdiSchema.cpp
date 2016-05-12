#include "pch.h"
#include "usdiInternal.h"
#include "usdiSchema.h"

namespace usdi {

Schema::Schema(Schema *parent)
    : m_parent(parent)
{
    if (m_parent) {
        m_parent->addChild(this);
    }
}

Schema::~Schema()
{
}

usdi::Schema* Schema::getParent()
{
    return m_parent;
}

size_t Schema::getNumChildren()
{
    return m_children.size();
}

Schema* Schema::getChild(int i)
{
    if ((size_t)i >= m_children.size()) { return nullptr; }
    return m_children[i];
}

const char* Schema::getPath()
{
    return getUSDPrim().GetPath().GetText();
}

const char* Schema::getName()
{
    return getUSDPrim().GetName().GetText();
}

UsdPrim Schema::getUSDPrim()
{
    return getUSDType().GetPrim();
}

SchemaType Schema::getType() const
{
    return SchemaType::Unknown;
}

void Schema::addChild(Schema *child)
{
    m_children.push_back(child);
}

} // namespace usdi
