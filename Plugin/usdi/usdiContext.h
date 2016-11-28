#pragma once

namespace usdi {

class Context
{
public:
    Context();
    virtual ~Context();

    bool                valid() const;
    void                initialize();
    bool                createStage(const char *identifier);
    bool                open(const char *path);
    bool                save() const;
    // path must *not* be same as identifier (parameter of createStage() or open())
    bool                saveAs(const char *path) const;

    const ImportConfig& getImportConfig() const;
    void                setImportConfig(const ImportConfig& v);
    const ExportConfig& getExportConfig() const;
    void                setExportConfig(const ExportConfig& v);

    UsdStageRefPtr      getUsdStage() const;
    Schema*             getRoot() const;
    int                 getNumMasters() const;
    Schema*             getMaster(int i) const;
    Schema*             findSchema(const char *path) const;

    // SchemaType: Xform, Camera, Mesh, etc
    template<class SchemaType>
    SchemaType*         createSchema(Schema *parent, const char *name);
    Schema*             createSchema(Schema *parent, const UsdPrim& prim);
    Schema*             createSchemaRecursive(Schema *parent, UsdPrim prim);
    Schema*             createInstanceSchema(Schema *parent, Schema *master, const std::string& path, UsdPrim prim);
    Schema*             createInstanceSchemaRecursive(Schema *parent, UsdPrim prim);
    Schema*             createOverride(const char *prim_path);
    void                flatten();

    void                beginEdit(const UsdEditTarget& t);
    void                endEdit();

    void                rebuildSchemaTree();
    int                 generateID();
    void                notifyForceUpdate();
    void                updateAllSamples(Time t);

private:
    void    addSchema(Schema *schema);
    void    applyImportConfig();

private:
    using SchemaPtr = std::unique_ptr<Schema>;
    using Schemas = std::vector<SchemaPtr>;
    using Masters = std::vector<Schema*>;

    UsdStageRefPtr  m_stage;
    Schemas         m_schemas;
    Schema*         m_root = nullptr;
    Masters         m_masters;

    ImportConfig    m_import_config;
    ExportConfig    m_export_config;

    int             m_id_seed = 0;
    double          m_start_time = 0.0;
    double          m_end_time = 0.0;
    UsdEditTarget   m_edit_target;
};

} // namespace usdi
