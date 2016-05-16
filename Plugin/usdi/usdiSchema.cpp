#include "pch.h"
#include "usdiInternal.h"
#include "usdiContext.h"
#include "usdiSchema.h"

namespace usdi {

Schema::Schema(Context *ctx, Schema *parent, const UsdTyped& usd_schema)
    : m_ctx(ctx)
    , m_parent(parent)
    , m_id(ctx->generateID())
    , m_prim(usd_schema.GetPrim())
{
    ctx->addSchema(this);
    if (m_parent) {
        m_parent->addChild(this);
    }
}

Schema::Schema(Context *ctx, Schema *parent, const char *name, const char *type)
    : m_ctx(ctx)
    , m_parent(parent)
    , m_id(ctx->generateID())
{
    ctx->addSchema(this);
    if (m_parent) {
        m_parent->addChild(this);
    }
    m_prim = ctx->getUSDStage()->DefinePrim(SdfPath(makePath(name)), TfToken(type));
}

Schema::~Schema()
{
}

const ImportConfig& Schema::getImportConfig() const { return m_ctx->getImportConfig(); }
const ExportConfig& Schema::getExportConfig() const { return m_ctx->getExportConfig(); }

Context*    Schema::getContext() const      { return m_ctx; }
int         Schema::getID() const           { return m_id; }
Schema*     Schema::getParent() const       { return m_parent; }
size_t      Schema::getNumChildren() const  { return m_children.size(); }
Schema*     Schema::getChild(int i) const   { return (size_t)i >= m_children.size() ? nullptr : m_children[i]; }

const char* Schema::getPath() const         { return getUSDPrim().GetPath().GetText(); }
const char* Schema::getName() const         { return getUSDPrim().GetName().GetText(); }
const char* Schema::getTypeName() const     { return getUSDPrim().GetTypeName().GetText(); }
UsdPrim     Schema::getUSDPrim() const      { return m_prim; }
UsdTyped    Schema::getUSDSchema() const    { return const_cast<Schema*>(this)->getUSDSchema(); }

void Schema::addChild(Schema *child)
{
    m_children.push_back(child);
}

std::string Schema::makePath(const char *name)
{
    std::string path;
    path += m_parent ? m_parent->getPath() : "/";
    path += name;
    return path;
}

} // namespace usdi
