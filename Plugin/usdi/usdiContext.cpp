#include "pch.h"
#include "usdiInternal.h"
#include "usdiSchema.h"
#include "usdiXform.h"
#include "usdiCamera.h"
#include "usdiMesh.h"
#include "usdiPoints.h"
#include "usdiContext.h"

namespace usdi {


Context::Context()
{
    usdiLog("Context::Context()\n");
}

Context::~Context()
{
    initialize();
    usdiLog("Context::~Context()\n");
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
    usdiLog("Context::create(): identifier %s\n", identifier);

    auto root_node = new Xform(this, nullptr, "root");
}


void Context::createNodeRecursive(Schema *parent, UsdPrim prim)
{
    if (!prim.IsValid()) { return; }

    Schema *node = createNode(parent, prim);

    auto children = prim.GetChildren();
    for (auto c : children) {
        createNodeRecursive(node, c);
    }
}

Schema* Context::createNode(Schema *parent, UsdPrim prim)
{
    Schema *ret = nullptr;

    UsdGeomPoints points(prim);
    UsdGeomMesh mesh(prim);
    UsdGeomCamera cam(prim);
    UsdGeomXform xf(prim);
    if (points) {
        ret = new Points(this, parent, points);
    }
    else if (mesh) {
        ret = new Mesh(this, parent, mesh);
    }
    else if (cam) {
        ret = new Camera(this, parent, cam);
    }
    else if (xf) {
        ret = new Xform(this, parent, xf);
    }

    return ret;
}

bool Context::open(const char *path)
{
    initialize();

    usdiLog("Context::open(): trying to open %s\n", path);

    ArGetResolver().ConfigureResolverForAsset(path);
    auto resolverctx = ArGetResolver().CreateDefaultContextForAsset(path);
    m_stage = UsdStage::Open(path, resolverctx);
    if (m_stage == UsdStageRefPtr()) {
        usdiLog("Context::open(): failed to load %s\n", path);
        return false;
    }

    m_start_time = m_stage->GetStartTimeCode();
    m_end_time = m_stage->GetEndTimeCode();

    //UsdPrim root_prim = m_stage->GetDefaultPrim();
    UsdPrim root_prim = m_stage->GetPseudoRoot();

    // Check if prim exists.  Exit if not
    if (!root_prim.IsValid()) {
        usdiLog("Context::open(): root prim is not valid\n");
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

UsdStageRefPtr Context::getUSDStage() const { return m_stage; }

void Context::addSchema(Schema *schema)
{
    m_schemas.emplace_back(schema);
}

int Context::generateID()
{
    return ++m_id_seed;
}

} // namespace usdi
