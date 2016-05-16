#include "pch.h"
#include "usdiInternal.h"
#include "usdiContext.h"
#include "usdiSchema.h"

namespace usdi {

Schema::Schema(Context *ctx, Schema *parent)
    : m_ctx(ctx)
    , m_parent(parent)
{
    if (m_parent) {
        m_parent->addChild(this);
    }
}

Schema::~Schema()
{
}

const ImportConfig& Schema::getImportConfig() const { return m_ctx->getImportConfig(); }
const ExportConfig& Schema::getExportConfig() const { return m_ctx->getExportConfig(); }

int         Schema::getID() const           { return m_id; }
Schema*     Schema::getParent() const       { return m_parent; }
size_t      Schema::getNumChildren() const  { return m_children.size(); }
Schema*     Schema::getChild(int i) const   { return (size_t)i >= m_children.size() ? nullptr : m_children[i]; }

const char* Schema::getPath() const         { return getUSDPrim().GetPath().GetText(); }
const char* Schema::getName() const         { return getUSDPrim().GetName().GetText(); }
const char* Schema::getTypeName() const     { return getUSDPrim().GetTypeName().GetText(); }
UsdPrim     Schema::getUSDPrim() const      { return getUSDSchema().GetPrim(); }
UsdTyped    Schema::getUSDSchema() const    { return const_cast<Schema*>(this)->getUSDSchema(); }

void Schema::setID(int id) { m_id = id; }

void Schema::addChild(Schema *child)
{
    m_children.push_back(child);
}

} // namespace usdi
