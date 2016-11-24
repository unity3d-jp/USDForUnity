#include "pch.h"
#include "usdiInternal.h"
#include "usdiSchema.h"
#include "usdiXform.h"
#include "usdiCamera.h"
#include "usdiMesh.h"
#include "usdiPoints.h"
#include "usdiContext.h"

void mDetachAllThreads();

namespace usdi {

#ifdef usdiEnableUnityExtension
    void InitializeInternalMethods();
    void ClearInternalMethodsCache();
#endif // usdiEnableUnityExtension

static int g_ctx_count;


Context::Context()
{
    ++g_ctx_count;
    if (g_ctx_count == 1) {
#ifdef usdiEnableUnityExtension
        usdi::InitializeInternalMethods();
#endif // usdiEnableUnityExtension
    }

    initialize();
    usdiLogTrace("Context::Context()\n");
}

Context::~Context()
{
    initialize();
    usdiLogTrace("Context::~Context()\n");
    --g_ctx_count;

#ifdef usdiEnableUnityExtension
    if (g_ctx_count == 0) {
        //mDetachAllThreads();
        usdi::ClearInternalMethodsCache();
    }
#endif // usdiEnableUnityExtension
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
    namespace fs = std::experimental::filesystem;

    initialize();

    FILE *f = fopen(identifier, "rb");
    if (f) {
        fclose(f);
        // UsdStage::CreateNew() will fail if file already exists. try to delete existing one.
        if (std::remove(identifier)) {
            usdiLogTrace("Context::createStage(): deleted existing file %s\n", identifier);
        }
    }
    else {
        // create output directory if not exist
        fs::path path = {identifier};
        if (path.has_parent_path()) {
            fs::create_directories(path.remove_filename());
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

bool Context::open(const char *path)
{
    initialize();

    usdiLogInfo( "Context::open(): %s\n", path);
    usdiLogTrace("  scale: %f\n", m_import_config.scale);
    usdiLogTrace("  triangulate: %d\n", (int)m_import_config.triangulate);
    usdiLogTrace("  swap_handedness: %d\n", (int)m_import_config.swap_handedness);
    usdiLogTrace("  swap_faces: %d\n", (int)m_import_config.swap_faces);

    m_stage = UsdStage::Open(path);
    if (!m_stage) {
        usdiLogWarning("Context::open(): failed to load %s\n", path);
        return false;
    }

    applyImportConfig();
    m_start_time = m_stage->GetStartTimeCode();
    m_end_time = m_stage->GetEndTimeCode();
    rebuildSchemaTree();
    return true;
}

bool Context::save() const
{
    if (!m_stage) {
        usdiLogError("Context::save(): m_stage is null\n");
        return false;
    }

    usdiLogInfo( "Context::save():\n");
    return m_stage->GetRootLayer()->Save();
}

bool Context::saveAs(const char *path) const
{
    if (!m_stage) {
        usdiLogError("Context::saveAs(): m_stage is null\n");
        return false;
    }

    {
        // UsdStage::Export() fail if dst file is not exist. workaround for it 
        FILE *f = fopen(path, "wb");
        if (f) {
            fclose(f);
        }
    }

    usdiLogInfo("Context::saveAs(): %s\n", path);
    return m_stage->Export(path);
}

const ImportConfig& Context::getImportConfig() const
{
    return m_import_config;
}

void Context::setImportConfig(const ImportConfig& v)
{
    if (v != m_import_config) {
        m_import_config = v;
        applyImportConfig();
        for (auto& s : m_schemas) { s->notifyImportConfigChanged(); }
    }
}

const ExportConfig& Context::getExportConfig() const
{
    return m_export_config;
}

void Context::setExportConfig(const ExportConfig& v)
{
    m_export_config = v;
}



UsdStageRefPtr Context::getUsdStage() const
{
    return m_stage;
}

Schema* Context::getRoot() const
{
    return m_root;
}

int Context::getNumMasters() const
{
    return (int)m_masters.size();
}

Schema* Context::getMaster(int i) const
{
    return m_masters[i];
}

Schema* Context::findSchema(const char *path) const
{
    // obviously this is very slow. I need to improve this if this method is called very often.
    for (auto& n : m_schemas) {
        if (strcmp(n->getPath(), path) == 0) {
            return n.get();
        }
    }
    return nullptr;
}


void Context::addSchema(Schema *schema)
{
    if (!schema) {
        usdiLogError("Context::addSchema(): invalid parameter\n");
        return;
    }
    usdiLogTrace("Context::addSchema(): %s\n", schema->getName());
    schema->setup();
    m_schemas.emplace_back(schema);
}

Schema* Context::createSchema(Schema *parent, const UsdPrim& prim)
{
    Schema *ret = CreateSchema(this, parent, prim);
    if (ret) {
        addSchema(ret);
    }
    return ret;
}

Schema* Context::createSchemaRecursive(Schema *parent, UsdPrim prim)
{
    if (!prim.IsValid()) { return nullptr; }

    auto *ret = createSchema(parent, prim);

    // handling payload
    if (m_import_config.load_all_payloads && prim.HasPayload()) {
        prim.Load();
    }

    // handling instance
    if (prim.IsInstance()) {
        auto children = prim.GetMaster().GetChildren();
        for (auto c : children) {
            createInstanceSchemaRecursive(ret, c);
        }
    }
    else {
        auto children = prim.GetChildren();
        for (auto c : children) {
            createSchemaRecursive(ret, c);
        }
    }
    return ret;
}

Schema* Context::createInstanceSchema(Schema *parent, Schema *master, const std::string& path, UsdPrim prim)
{
    auto *ret = new Schema(this, parent, master, path, prim);
    addSchema(ret);
    return ret;
}

Schema* Context::createInstanceSchemaRecursive(Schema *parent, UsdPrim prim)
{
    Schema *master = findSchema(prim.GetPath().GetText());

    std::string path = parent ? parent->getPath() : "/";
    if (path.back() != '/') {
        path += '/';
    }
    path += prim.GetName();

    auto *ret = createInstanceSchema(parent, master, path, prim);
    auto children = prim.GetChildren();
    for (auto c : children) {
        createInstanceSchemaRecursive(ret, c);
    }
    return ret;
}

Schema* Context::createOverride(const char *prim_path)
{
    if (!prim_path) {
        usdiLogError("Context::createOverride(): invalid parameter\n");
        return nullptr;
    }

    if (auto *p = findSchema(prim_path)) {
        return p;
    }

    if (auto prim = m_stage->OverridePrim(SdfPath(prim_path))) {
        auto *ret = new Schema(this, nullptr, prim);
        addSchema(ret);
        return ret;
    }
    return nullptr;
}

void Context::flatten()
{
    if (!m_stage) {
        usdiLogError("Context::flatten(): m_stage is null\n");
        return;
    }

    m_stage->Flatten();
}


void Context::beginEdit(const UsdEditTarget& t)
{
    m_edit_target = m_stage->GetEditTarget();
    m_stage->SetEditTarget(t);
}
void Context::endEdit()
{
    m_stage->SetEditTarget(m_edit_target);
}

void Context::rebuildSchemaTree()
{
    m_masters.clear();
    m_schemas.clear();
    m_root = nullptr;
    m_id_seed = 0;

    {
        auto masters = m_stage->GetMasters();
        for (auto& m : masters) {
            m_masters.push_back(createSchemaRecursive(nullptr, m));
        }
    }
    {
        auto root_prim = m_stage->GetPseudoRoot();
        if (root_prim.IsValid()) {
            m_root = createSchemaRecursive(nullptr, root_prim);
        }
    }
}

int Context::generateID()
{
    return ++m_id_seed;
}

void Context::updateAllSamples(Time t)
{
#ifdef usdiDbgForceSingleThread
    for (auto& s : m_schemas) {
        s->updateSample(t);
    }
#else
    size_t grain = std::max<size_t>(m_schemas.size() / 32, 1);
    tbb::parallel_for(tbb::blocked_range<size_t>(0, m_schemas.size(), grain), [t, this](const auto& r) {
        for (size_t i = r.begin(); i != r.end(); ++i) {
            m_schemas[i]->updateSample(t);
        }
    });
#endif
}

} // namespace usdi
