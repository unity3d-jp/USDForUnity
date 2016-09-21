#pragma once

namespace usdi {

class Schema
{
public:
    Schema(Context *ctx, Schema *parent, const UsdTyped& usd_schema); // for import
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

    const char*         getPath() const;
    const char*         getName() const;
    const char*         getTypeName() const;
    void                getTimeRange(Time& start, Time& end) const;
    UsdPrim             getUSDPrim() const;
    UsdTyped            getUSDSchema() const;
    virtual UsdTyped&   getUSDSchema() = 0;

    virtual void        updateSample(Time t);

public: // for internal use
    void addChild(Schema *child);
    std::string makePath(const char *name);

    const ImportConfig& getImportConfig() const;
    const ExportConfig& getExportConfig() const;

protected:
    typedef std::vector<Schema*> Children;
    typedef std::unique_ptr<Attribute> AttributePtr;
    typedef std::vector<AttributePtr> Attributes;

    bool needsUpdate(Time t) const;

    Context     *m_ctx;
    Schema      *m_parent;
    UsdPrim     m_prim;
    Children    m_children;
    Attributes  m_attributes;
    int         m_id = 0;
    Time        m_time_start = usdiInvalidTime, m_time_end = usdiInvalidTime;
    Time        m_prev_time = usdiInvalidTime;
#ifdef usdiDebug
    const char *m_dbg_path = nullptr;
    const char *m_dbg_typename = nullptr;
#endif
};

} // namespace usdi
