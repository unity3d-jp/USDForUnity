#include "pch.h"
#include "usdiInternal.h"
#include "usdiSchema.h"
#include "usdiUnknown.h"
#include "usdiXform.h"
#include "usdiCamera.h"
#include "usdiMesh.h"
#include "usdiPoints.h"
#include "usdiContext.h"

namespace usdi {


Context::Context()
{
    initialize();
    usdiLogTrace("Context::Context()\n");
}

Context::~Context()
{
    initialize();
    usdiLogTrace("Context::~Context()\n");
}

bool Context::valid() const
{
    return m_stage;
}

void Context::initialize()
{
    if (m_stage) {
        m_stage->Close();
    }
    m_stage = UsdStageRefPtr();

    // delete USD objects in reverse order
    for (auto i = m_schemas.rbegin(); i != m_schemas.rend(); ++i) { i->reset(); }
    m_schemas.clear();

    m_id_seed = 0;
    m_start_time = 0.0;
    m_end_time = 0.0;
}

bool Context::createStage(const char *identifier)
{
    initialize();

    // UsdStage::CreateNew() will fail if file already exists. try to delete existing one.
    {
        FILE *f = fopen(identifier, "rb");
        if (f) {
            fclose(f);
            if (std::remove(identifier)) {
                usdiLogTrace("Context::createStage(): deleted existing file %s\n", identifier);
            }
        }
    }

    m_stage = UsdStage::CreateNew(identifier);
    if (m_stage) {
        usdiLogInfo("Context::createStage(): succeeded to create %s\n", identifier);
    }
    else {
        usdiLogInfo("Context::createStage(): failed to create %s\n", identifier);
    }
    return m_stage;
}


void Context::applyImportConfig()
{
    if (!m_stage) { return; }

    switch (m_import_config.interpolation) {
    case InterpolationType::None: m_stage->SetInterpolationType(UsdInterpolationTypeHeld); break;
    case InterpolationType::Linear: m_stage->SetInterpolationType(UsdInterpolationTypeLinear); break;
    }
}

void Context::createNodeRecursive(Schema *parent, UsdPrim prim, int depth)
{
    if (!prim.IsValid()) { return; }

    Schema *node = createNode(parent, prim);
    auto children = prim.GetChildren();
    for (auto c : children) {
        createNodeRecursive(node, c, depth + 1);
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
    else {
        ret = new Unknown(this, parent, xf);
    }

    return ret;
}

bool Context::open(const char *path)
{
    initialize();

    usdiLogInfo( "Context::open(): %s\n", path);
    usdiLogTrace("  scale: %f\n", m_import_config.scale);
    usdiLogTrace("  triangulate: %d\n", (int)m_import_config.triangulate);
    usdiLogTrace("  swap_handedness: %d\n", (int)m_import_config.swap_handedness);
    usdiLogTrace("  swap_faces: %d\n", (int)m_import_config.swap_faces);

    ArGetResolver().ConfigureResolverForAsset(path);
    auto resolverctx = ArGetResolver().CreateDefaultContextForAsset(path);
    m_stage = UsdStage::Open(path, resolverctx);
    if (!m_stage) {
        usdiLogWarning("Context::open(): failed to load %s\n", path);
        return false;
    }

    applyImportConfig();
    m_start_time = m_stage->GetStartTimeCode();
    m_end_time = m_stage->GetEndTimeCode();

    //UsdPrim root_prim = m_stage->GetDefaultPrim();
    UsdPrim root_prim = m_stage->GetPseudoRoot();

    // Check if prim exists.  Exit if not
    if (!root_prim.IsValid()) {
        usdiLogWarning("Context::open(): root prim is not valid\n");
        initialize();
        return false;
    }

    // Set the variants on the usdRootPrim
    for (auto it = m_variants.begin(); it != m_variants.end(); ++it) {
        root_prim.GetVariantSet(it->first).SetVariantSelection(it->second);
    }

    createNodeRecursive(nullptr, root_prim, 0);
    return true;
}

bool Context::save()
{
    usdiLogInfo( "Context::save():\n");
    usdiLogTrace("  scale: %f\n", m_export_config.scale);
    usdiLogTrace("  swap_handedness: %d\n", (int)m_export_config.swap_handedness);
    usdiLogTrace("  swap_faces: %d\n", (int)m_export_config.swap_faces);

    return m_stage->GetRootLayer()->Save();
}

bool Context::write(const char *path)
{
    usdiLogInfo( "Context::write(): %s\n", path);
    usdiLogTrace("  scale: %f\n", m_export_config.scale);
    usdiLogTrace("  swap_handedness: %d\n", (int)m_export_config.swap_handedness);
    usdiLogTrace("  swap_faces: %d\n", (int)m_export_config.swap_faces);

    return m_stage->Export(path);
}

const ImportConfig& Context::getImportConfig() const
{
    return m_import_config;
}

void Context::setImportConfig(const ImportConfig& v)
{
    m_import_config = v;
    applyImportConfig();
}

const ExportConfig& Context::getExportConfig() const
{
    return m_export_config;
}

void Context::setExportConfig(const ExportConfig& v)
{
    m_export_config = v;
}


Schema* Context::getRootNode()
{
    return m_schemas.empty() ? nullptr : m_schemas.front().get();
}

UsdStageRefPtr Context::getUSDStage() const { return m_stage; }

void Context::addSchema(Schema *schema)
{
    if (!schema) { return; }
    usdiLogTrace("Context::addSchema(): %s\n", schema->getName());
    m_schemas.emplace_back(schema);
}

int Context::generateID()
{
    return ++m_id_seed;
}

void Context::updateAllSamples(Time t)
{
#ifdef usdiDebug
    for (auto& s : m_schemas) {
        s->updateSample(t);
    }
#else
    tbb::parallel_for(tbb::blocked_range<size_t>(0, m_schemas.size()), [t, this](const auto& r) {
        for (size_t i = r.begin(); i != r.end(); ++i) {
            m_schemas[i]->updateSample(t);
        }
    }, tbb::auto_partitioner());
#endif
}

} // namespace usdi
