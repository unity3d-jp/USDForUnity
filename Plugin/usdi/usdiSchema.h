#pragma once

namespace usdi {

class Schema
{
public:
    Schema(Context *ctx, const UsdPrim& p);
    Schema(Context *ctx, Schema *parent, const UsdSchemaBase& usd_schema); // for import
    Schema(Context *ctx, Schema *parent, const char *name, const char *type); // for export
    void init();
    virtual ~Schema();

    Context*            getContext() const;
    int                 getID() const;
    Schema*             getParent() const;
    size_t              getNumChildren() const;
    Schema*             getChild(int i) const;

    size_t              getNumAttributes() const;
    Attribute*          getAttribute(int i) const;
    Attribute*          findAttribute(const char *name) const;
    Attribute*          createAttribute(const char *name, AttributeType type);

    void                setInstanceable(bool v);
    bool                isInstance() const;
    Schema*             getMaster() const;
    bool                isMaster() const;

    const char*         getPath() const;
    const char*         getName() const;
    const char*         getTypeName() const;
    void                getTimeRange(Time& start, Time& end) const;
    UsdPrim             getUSDPrim() const;
    UsdSchemaBase&      getUSDSchema() const;
    virtual UsdSchemaBase& getUSDSchema();

    bool                needsUpdate() const;
    virtual void        updateSample(Time t);
    virtual void        invalidateSample();

    void                setUserData(void *v);
    void*               getUserData() const;

    template<class Body>
    void each(const Body& body)
    {
        for (auto& c : m_children) { body(c); }
    }


public: // for internal use
    void addChild(Schema *child);
    std::string makePath(const char *name);

    const ImportConfig& getImportConfig() const;
    const ExportConfig& getExportConfig() const;

protected:
    typedef std::vector<Schema*> Children;
    typedef std::unique_ptr<Attribute> AttributePtr;
    typedef std::vector<AttributePtr> Attributes;


    Context         *m_ctx = nullptr;
    Schema          *m_parent = nullptr;
    UsdPrim         m_prim;
    UsdSchemaBase   m_usd_schema;
    Children        m_children;
    Attributes      m_attributes;
    int             m_id = 0;
    Time            m_time_start = usdiInvalidTime, m_time_end = usdiInvalidTime;
    Time            m_time_prev = usdiInvalidTime;
    bool            m_needs_update = true;
    void            *m_userdata = nullptr;
#ifdef usdiDebug
    const char *m_dbg_path = nullptr;
    const char *m_dbg_typename = nullptr;
#endif
};

} // namespace usdi
