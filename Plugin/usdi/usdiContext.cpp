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

    auto root_prim = m_stage->GetPseudoRoot();
    if (!root_prim.IsValid()) {
        usdiLogWarning("Context::open(): root prim is not valid\n");
        initialize();
        return false;
    }

    createSchemaRecursive(nullptr, root_prim);
    return true;
}

bool Context::save()
{
    if (!m_stage) {
        usdiLogError("Context::save(): m_stage is null\n");
        return false;
    }

    usdiLogInfo( "Context::save():\n");
    return m_stage->GetRootLayer()->Save();
}

bool Context::saveAs(const char *path)
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


Schema* Context::getRootSchema()
{
    return m_schemas.empty() ? nullptr : m_schemas.front().get();
}

Schema* Context::findSchema(const char *path)
{
    // obviously this is very slow. I need to improve this if this method is called very often.
    for (auto& n : m_schemas) {
        if (strcmp(n->getPath(), path) == 0) {
            return n.get();
        }
    }
    return nullptr;
}

UsdStageRefPtr Context::getUSDStage() const { return m_stage; }

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

void Context::invalidateAllSamples()
{
    for (auto& s : m_schemas) {
        s->invalidateSample();
    }
}

void Context::addSchema(Schema *schema)
{
    if (!schema) {
        usdiLogError("Context::addSchema(): invalid parameter\n");
        return;
    }
    schema->setup();
    usdiLogTrace("Context::addSchema(): %s\n", schema->getName());
    m_schemas.emplace_back(schema);
}

template<class T>
T* Context::createSchema(Schema *parent, const char *name)
{
    T *ret = new T(this, parent, name);
    addSchema(ret);
    if (parent) { parent->addChild(ret); }
    return ret;
}
template<class T>
T* Context::createSchema(Schema *parent, const UsdPrim& t)
{
    T *ret = new T(this, parent, t);
    addSchema(ret);
    if (parent) { parent->addChild(ret); }
    return ret;
}
#define Instanciate(T)\
    template T* Context::createSchema<T>(Schema *parent, const char *name);\
    template T* Context::createSchema<T>(Schema *parent, const UsdPrim& t);
Instanciate(Xform);
Instanciate(Camera);
Instanciate(Mesh);
Instanciate(Points);
#undef Instanciate

Schema* Context::createSchema(Schema *parent, UsdPrim prim)
{
    Schema *ret = nullptr;

    const auto& tn = prim.GetTypeName();
    if (tn == Xform::UsdTypeName) {
        ret = new Xform(this, parent, prim);
    }
    else if (tn == Points::UsdTypeName) {
        ret = new Points(this, parent, prim);
    }
    else if (tn == Mesh::UsdTypeName) {
        ret = new Mesh(this, parent, prim);
    }
    else if (tn == Camera::UsdTypeName) {
        ret = new Camera(this, parent, prim);
    }
    else {
        ret = new Schema(this, parent, prim);
    }

    if (ret) {
        addSchema(ret);
        if (parent) { parent->addChild(ret); }
    }
    return ret;
}

Schema* Context::createSchema(Schema *parent, Schema *master, const std::string& path, UsdPrim prim)
{
    auto *ret = new Schema(this, parent, master, path, prim);
    addSchema(ret);
    if (parent) { parent->addChild(ret); }
    return ret;
}

Schema* Context::createSchemaRecursive(Schema *parent, UsdPrim prim)
{
    if (!prim.IsValid()) { return nullptr; }

    auto *ret = createSchema(parent, prim);
    if (prim.IsInstance()) {
        if (!findSchema(prim.GetMaster().GetPath().GetText())) {
            createSchemaRecursive(nullptr, prim.GetMaster());
        }
        auto children = prim.GetMaster().GetChildren();
        for (auto c : children) {
            createReferenceSchemaRecursive(ret, c);
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

Schema* Context::createReferenceSchemaRecursive(Schema *parent, UsdPrim prim)
{
    Schema *master = findSchema(prim.GetPath().GetText());

    std::string path = parent ? parent->getPath() : "/";
    if (path.back() != '/') {
        path += '/';
    }
    path += prim.GetName();

    auto *ret = createSchema(parent, master, path, prim);
    auto children = prim.GetChildren();
    for (auto c : children) {
        createReferenceSchemaRecursive(ret, c);
    }
    return ret;
}

Schema* Context::createReference(const char *dstprim, const char *assetpath, const char *srcprim)
{
    if (!m_stage ) {
        usdiLogError("Context::createReference(): m_stage is null\n");
        return nullptr;
    }
    if (!dstprim || !srcprim) {
        usdiLogError("Context::createReference(): invalid parameter\n");
        return nullptr;
    }

    if (!assetpath) { assetpath = ""; }
    if (auto prim = m_stage->OverridePrim(SdfPath(dstprim))) {
        if (prim.GetReferences().Add(SdfReference(assetpath, SdfPath(srcprim)))) {
            // created successfully
            auto *ret = findSchema(dstprim);
            if (!ret) {
                ret = new Schema(this, nullptr, prim);
                addSchema(ret);
            }
            return ret;
        }
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

} // namespace usdi
