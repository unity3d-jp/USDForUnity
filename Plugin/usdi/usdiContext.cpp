#include "pch.h"
#include "usdiInternal.h"
#include "usdiSchema.h"
#include "usdiXform.h"
#include "usdiMesh.h"
#include "usdiContext.h"

namespace usdi {


Context::Context()
{
}

Context::~Context()
{
    initialize();
}

bool Context::valid() const
{
    return m_stage;
}

void Context::initialize()
{
    m_stage = UsdStageRefPtr();
    m_schemas.clear();

    m_id_seed = 0;
    m_start_time = 0.0;
    m_end_time = 0.0;
}

void Context::create(const char *identifier)
{
    initialize();

    m_stage = UsdStage::CreateNew(identifier);
}


void Context::createNodeRecursive(Schema *parent, UsdPrim prim)
{
    Schema *node = createNode(parent, prim);
    if (node) {
        m_schemas.emplace_back(node);
    }

    auto children = prim.GetChildren();
    for (auto c : children) {
        createNodeRecursive(node, c);
    }
}

Schema* Context::createNode(Schema *parent, UsdPrim prim)
{
    Schema *ret = nullptr;

    UsdGeomMesh mesh(prim);
    UsdGeomXform xf(prim);
    if (mesh) {
        ret = new Mesh(this, parent, mesh);
    }
    else if (xf) {
        ret = new Xform(this, parent, xf);
    }

    if (ret) {
        ret->setID(++m_id_seed);
    }
    return ret;
}

bool Context::open(const char *path)
{
    initialize();

    m_stage = UsdStage::Open(path);
    if (m_stage == UsdStageRefPtr()) {
        usdiLog("failed to load %s\n", path);
        return false;
    }

    m_start_time = m_stage->GetStartTimeCode();
    m_end_time = m_stage->GetEndTimeCode();

    UsdPrim root_prim = m_prim_root.empty() ? m_stage->GetDefaultPrim() : m_stage->GetPrimAtPath(SdfPath(m_prim_root));
    if (!root_prim && !(m_prim_root.empty() || m_prim_root == "/")) {
        root_prim = m_stage->GetPseudoRoot();
    }

    // Check if prim exists.  Exit if not
    if (!root_prim.IsValid()) {
        usdiLog("root prim is not valid\n");
        initialize();
        return false;
    }

    // Set the variants on the usdRootPrim
    for (auto it = m_variants.begin(); it != m_variants.end(); ++it) {
        root_prim.GetVariantSet(it->first).SetVariantSelection(it->second);
    }

    createNodeRecursive(nullptr, root_prim);
}

bool Context::write(const char *path)
{
    m_stage->Export(path);
    // todo
    return false;
}

const ImportConfig& Context::getImportConfig() const                { return m_import_config; }
void                Context::setImportConfig(const ImportConfig& v) { m_import_config = v; }
const ExportConfig& Context::getExportConfig() const                { return m_export_config; }
void                Context::setExportConfig(const ExportConfig& v) { m_export_config = v; }


Schema* Context::getRootNode()
{
    return m_schemas.empty() ? nullptr : m_schemas.front().get();
}

} // namespace usdi
