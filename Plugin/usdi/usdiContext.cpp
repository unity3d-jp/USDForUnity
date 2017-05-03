#include "pch.h"
#include "usdiInternal.h"
#include "usdiSchema.h"
#include "usdiXform.h"
#include "usdiCamera.h"
#include "usdiMesh.h"
#include "usdiPoints.h"
#include "usdiContext.h"
#include "usdiUtils.h"
#include "usdiRT/usdiRT.h"
#include "etc/tls.h"

void mDetachAllThreads();

namespace usdi {


namespace fs =
#ifdef usdiEnableBoostFilesystem
    boost::filesystem;
#else
    std::experimental::filesystem;
#endif

#define UsdSearchPathName "PXR_AR_DEFAULT_SEARCH_PATH"

static int g_ctx_count;


Context::Context()
{
    ++g_ctx_count;

    initialize();
    usdiLogTrace("Context::Context()\n");
}

Context::~Context()
{
    initialize();
    usdiLogTrace("Context::~Context()\n");
    --g_ctx_count;
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
        rebuildSchemaTree();
    }
    else {
        usdiLogInfo("Context::createStage(): failed to create %s\n", identifier);
    }
    return m_stage;
}


void Context::applyImportConfig()
{
    if (!m_stage) { return; }

    switch (m_import_settings.interpolation) {
    case InterpolationType::None: m_stage->SetInterpolationType(UsdInterpolationTypeHeld); break;
    case InterpolationType::Linear: m_stage->SetInterpolationType(UsdInterpolationTypeLinear); break;
    }
}

void Context::addAssetSearchPath(const char *path)
{
    if (!path) { return; }

    auto *search_paths_ = GetEnv(UsdSearchPathName);
    std::string search_paths = search_paths_ ? search_paths_ : "";

    fs::path fp{ path };
    if (fp.has_extension()) {
        fp.remove_filename();
    }
    auto str = fp.string();

    if (search_paths.find(str.c_str(), 0, str.size()) == std::string::npos) {
        if (!search_paths.empty()) {
            search_paths += ":";
        }
        search_paths += str;
        SetEnv(UsdSearchPathName, search_paths.c_str());
    }
}

void Context::clearAssetSearchPath()
{
    SetEnv(UsdSearchPathName, "");
}

bool Context::convertUSDToAlembic(const char *src_usd, const char *dst_abc)
{
    if (auto stage = UsdStage::Open(src_usd)) {
        if (auto layer = stage->GetRootLayer()) {
            return SdfFileFormat::FindByExtension(".abc")->WriteToFile(boost::get_pointer(layer), dst_abc);
        }
    }
    return false;
}


bool Context::open(const char *path)
{
    initialize();

    usdiLogInfo( "Context::open(): %s\n", path);

    // set USD's asset resolve paths
    addAssetSearchPath(path);

    m_stage = UsdStage::Open(path);
    clearAssetSearchPath();
    if (!m_stage) {
        // first try to open .abc often fails (likely Windows-only problem)
        // try again for workaround.
        m_stage = UsdStage::Open(path);
        if (!m_stage) {
            usdiLogWarning("Context::open(): failed to load %s\n", path);
            return false;
        }
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

const ImportSettings& Context::getImportSettings() const
{
    return m_import_settings;
}

void Context::setImportSettings(const ImportSettings& v)
{
    if (v != m_import_settings) {
        m_import_settings = v;
        applyImportConfig();
        for (auto& s : m_schemas) { s->notifyImportConfigChanged(); }
    }
}

const ExportSettings& Context::getExportSettings() const
{
    return m_export_settings;
}

void Context::setExportSettings(const ExportSettings& v)
{
    m_export_settings = v;
}



UsdStageRefPtr  Context::getUsdStage() const    { return m_stage; }
Schema*         Context::getRoot() const        { return m_root; }
int             Context::getNumSchemas() const  { return (int)m_schemas.size(); }
Schema*         Context::getSchema(int i) const { return m_schemas[i].get(); }
int             Context::getNumMasters() const  { return (int)m_masters.size(); }
Schema*         Context::getMaster(int i) const { return m_masters[i]; }

Schema* Context::findSchema(const char *path) const
{
    if (auto *p = FindSchema(m_masters, path)) { return p; }
    if (auto *p = FindSchema(m_schemas, path)) { return p; }
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
    if (m_import_settings.load_all_payloads && prim.HasPayload()) {
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
    m_edit_targets.push_back(m_stage->GetEditTarget());
    m_stage->SetEditTarget(t);
}
void Context::endEdit()
{
    if (!m_edit_targets.empty()) {
        auto t = m_edit_targets.back();
        m_stage->SetEditTarget(t);
        m_edit_targets.pop_back();
    }
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

void Context::notifyForceUpdate()
{
    for (auto& s : m_schemas) {
        s->notifyForceUpdate();
    }
}

void Context::updateAllSamples(Time t)
{
#ifdef usdiDbgForceSingleThread
    for (auto& s : m_schemas) {
        s->updateSample(t);
    }
#else
    size_t grain = std::max<size_t>(m_schemas.size() / 32, 1);
    using range_t = tbb::blocked_range<size_t>;
    tbb::parallel_for(range_t(0, m_schemas.size(), grain), [t, this](const range_t& r) {
        for (size_t i = r.begin(); i != r.end(); ++i) {
            m_schemas[i]->updateSample(t);
        }
    });
#endif
}

int Context::eachTimeSample(const TimeSampleCallback& cb)
{
    using Times = std::set<Time>;
    tls<Times> times;

    auto gather_times = [&times](Schema *s) {
        if (s->getMaster() != nullptr) { return; }

        auto& set = times.local();
        if (Xform *xf = usdiAsXform(s)) {
            xf->eachSample([&set](const XformData&, Time t) {
                set.insert(t);
            });
        }
        if (Camera *cam = usdiAsCamera(s)) {
            cam->eachSample([&set](const CameraData&, Time t) {
                set.insert(t);
            });
        }
        if (Mesh *mesh = usdiAsMesh(s)) {
            mesh->eachSample([&set](const MeshData&, Time t) {
                set.insert(t);
            });
        }
        if (Points *points = usdiAsPoints(s)) {
            points->eachSample([&set](const PointsData&, Time t) {
                set.insert(t);
            });
        }
    };

    size_t grain = std::max<size_t>(m_schemas.size() / 32, 1);
    using range_t = tbb::blocked_range<size_t>;
    tbb::parallel_for(range_t(0, m_schemas.size(), grain), [this, &gather_times](const range_t& r) {
        for (size_t i = r.begin(); i != r.end(); ++i) {
            gather_times(m_schemas[i].get());
        }
    });

    Times merged;
    times.each([&merged](Times& t) {
        merged.insert(t.begin(), t.end());
    });

    for (Time t : merged) {
        cb(t);
    }
    return 0;
}

static void precomputeNormalsAllImpl(Schema *schema, bool gen_tangents, bool overwrite, const Context::precomputeNormalsCallback& cb)
{
    if (!schema) { return; }

    schema->eachChild([&](Schema *s) {
        s->editVariants([&]() {
            auto *mesh = dynamic_cast<Mesh*>(s);
            if (mesh && mesh->isEditable()) {
                bool done = mesh->precomputeNormals(gen_tangents, overwrite);
                if (cb) { cb(mesh, done); }
            }
            precomputeNormalsAllImpl(s, gen_tangents, overwrite, cb);
        });
    });
}
void Context::precomputeNormalsAll(bool gen_tangents, bool overwrite, const precomputeNormalsCallback& cb)
{
    precomputeNormalsAllImpl(getRoot(), gen_tangents, overwrite, cb);
}

} // namespace usdi
