#include "pch.h"
#include "usdiInternal.h"
#include "usdiContext.h"
#include "usdiAttribute.h"
#include "usdiSchema.h"

namespace usdi {

Schema::Schema(Context *ctx, Schema *parent, const UsdTyped& usd_schema)
    : m_ctx(ctx)
    , m_parent(parent)
    , m_prim(usd_schema.GetPrim())
    , m_id(ctx->generateID())
{
    init();
}

Schema::Schema(Context *ctx, Schema *parent, const char *name, const char *type)
    : m_ctx(ctx)
    , m_parent(parent)
    , m_id(ctx->generateID())
{
    m_prim = ctx->getUSDStage()->DefinePrim(SdfPath(makePath(name)), TfToken(type));
    init();
}

void Schema::init()
{
    m_ctx->addSchema(this);
    if (m_parent) {
        m_parent->addChild(this);
    }

    if (m_prim) {
        // gather attributes
        auto attrs = m_prim.GetAuthoredAttributes();
        for (auto attr : attrs) {
            if (auto *ret = WrapExistingAttribute(this, attr)) {
                m_attributes.emplace_back(ret);
            }
        }
#ifdef usdiDebug
        m_dbg_path = getPath();
        m_dbg_typename = getTypeName();
#endif
    }

    {
        // get start & end time
        double lower = 0.0, upper = 0.0;
        bool first = true;
        for (auto& a : m_attributes) {
            double l, u;
            if (a->getTimeRange(l, u)) {
                if (first) {
                    lower = l;
                    upper = u;
                    first = false;
                }
                else {
                    lower = std::min(lower, l);
                    upper = std::max(upper, u);
                }
            }
        }
        m_time_start = lower;
        m_time_end = upper;
    }
}

Schema::~Schema()
{
    m_attributes.clear();
}

Context*    Schema::getContext() const      { return m_ctx; }
int         Schema::getID() const           { return m_id; }

bool Schema::isInstance() const
{
    return m_prim.IsInstance();
}

Schema* Schema::getMaster() const
{
    return m_ctx->getNodeByPath(m_prim.GetMaster().GetPath().GetText());
}


bool Schema::isMaster() const
{
    return m_prim.IsMaster();
}

Schema*     Schema::getParent() const       { return m_parent; }
size_t      Schema::getNumChildren() const  { return m_children.size(); }
Schema*     Schema::getChild(int i) const   { return (size_t)i >= m_children.size() ? nullptr : m_children[i]; }


size_t      Schema::getNumAttributes() const { return m_attributes.size(); }
Attribute*  Schema::getAttribute(int i) const { return (size_t)i >= m_attributes.size() ? nullptr : m_attributes[i].get(); }

Attribute* Schema::findAttribute(const char *name) const
{
    for (const auto& a : m_attributes) {
        if (strcmp(a->getName(), name) == 0) {
            return a.get();
        }
    }
    return nullptr;
}

Attribute* Schema::createAttribute(const char *name, AttributeType type)
{
    if (auto *f = findAttribute(name)) { return f; }

    if (auto *c = CreateNewAttribute(this, name, type)) {
        m_attributes.emplace_back(c);
        return c;
    }
    return nullptr;
}

const char* Schema::getPath() const         { return m_prim.GetPath().GetText(); }
const char* Schema::getName() const         { return m_prim.GetName().GetText(); }
const char* Schema::getTypeName() const     { return m_prim.GetTypeName().GetText(); }

void Schema::getTimeRange(Time& start, Time& end) const
{
    start = m_time_start;
    end = m_time_end;
}

UsdPrim     Schema::getUSDPrim() const      { return m_prim; }
UsdTyped    Schema::getUSDSchema() const    { return const_cast<Schema*>(this)->getUSDSchema(); }

bool Schema::needsUpdate() const
{
    return m_needs_update;
}

void Schema::updateSample(Time t)
{
    m_needs_update = true;
    if (m_time_prev != usdiInvalidTime) {
        if (t == m_time_prev) { m_needs_update = false; }
        else if ((t <= m_time_start && m_time_prev <= m_time_start) || (t >= m_time_end && m_time_prev >= m_time_end)) {
            m_needs_update = false;
        }
    }
    m_time_prev = t;

    //for (auto& a : m_attributes) {
    //    a->updateSample(t);
    //}
}

void Schema::invalidateSample()
{
    m_needs_update = true;
    m_time_prev = usdiInvalidTime;
}

void Schema::setUserData(void *v) { m_userdata = v; }
void* Schema::getUserData() const { return m_userdata; }


const ImportConfig& Schema::getImportConfig() const { return m_ctx->getImportConfig(); }
const ExportConfig& Schema::getExportConfig() const { return m_ctx->getExportConfig(); }

void Schema::addChild(Schema *child)
{
    m_children.push_back(child);
}

std::string Schema::makePath(const char *name_)
{
    // sanitize
    std::string name = name_;
    for (auto& c : name) {
        if (!std::isalnum(c)) {
            c = '_';
        }
    }

    std::string path;
    if (m_parent) {
        path += m_parent->getPath();
    }
    path += "/";
    path += name;

    usdiLogTrace("Schema::makePath(): %s\n", path.c_str());
    return path;
}

} // namespace usdi
