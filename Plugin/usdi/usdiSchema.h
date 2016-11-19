#pragma once

namespace usdi {

class Schema
{
friend class Context;
protected:
    Schema(Context *ctx, Schema *parent, Schema *master, const std::string& path, const UsdPrim& p);
    Schema(Context *ctx, Schema *parent, const UsdPrim& p);
    Schema(Context *ctx, Schema *parent, const char *name, const char *type); // for export
    void init();
    virtual void setup();
public:
    virtual ~Schema();

    Context*        getContext() const;
    int             getID() const;
    Schema*         getParent() const;
    int             getNumChildren() const;
    Schema*         getChild(int i) const;

    int             getNumAttributes() const;
    Attribute*      getAttribute(int i) const;
    Attribute*      findAttribute(const char *name) const;
    Attribute*      createAttribute(const char *name, AttributeType type);

    int             getNumVariantSets() const;
    const char*     getVariantSetName(int iset) const;
    int             getNumVariants(int iset) const;
    const char*     getVariantName(int iset, int ival) const;
    bool            setVariantSelection(int iset, int ival);
    // return -1 if fail
    int             findVariantSet(const char *name) const;
    // return -1 if fail
    int             findVariant(int iset, const char *name) const;
    // return index of added set. -1 if fail
    int             createVariantSet(const char *name);
    // return index of added variant. -1 if fail
    int             createVariant(int iset, const char *name);

    Schema*         getMaster() const;
    bool            isInstance() const;
    bool            isInstanceable() const;
    bool            isMaster() const;
    void            setInstanceable(bool v);

    const char*     getPath() const;
    const char*     getName() const;
    const char*     getTypeName() const;
    void            getTimeRange(Time& start, Time& end) const;
    UsdPrim         getUSDPrim() const;

    bool            needsUpdate() const;
    virtual void    updateSample(Time t);
    virtual void    invalidateSample();

    void            setUserData(void *v);
    void*           getUserData() const;

    template<class Body>
    void each(const Body& body)
    {
        for (auto& c : m_children) { body(c); }
    }

    template<class T>
    T as()
    {
        if (auto *m = getMaster()) {
            return dynamic_cast<T>(m);
        }
        else {
            return dynamic_cast<T>(this);
        }
    }


protected:
    void addChild(Schema *child);
    std::string makePath(const char *name);

    const ImportConfig& getImportConfig() const;
    const ExportConfig& getExportConfig() const;

protected:
    typedef std::vector<Schema*> Children;
    typedef std::unique_ptr<Attribute> AttributePtr;
    typedef std::vector<AttributePtr> Attributes;

    struct VariantSet
    {
        std::string name;
        std::vector<std::string> variants;
    };

    void syncAttributes();
    void syncVariantSets();

    Context         *m_ctx = nullptr;
    Schema          *m_parent = nullptr;
    Schema          *m_master = nullptr;
    int             m_id = 0;

    std::string     m_path;
    UsdPrim         m_prim;
    Children        m_children;
    Attributes      m_attributes;

    std::vector<VariantSet> m_variant_sets;

    Time            m_time_start = usdiInvalidTime, m_time_end = usdiInvalidTime;
    Time            m_time_prev = usdiInvalidTime;
    bool            m_needs_update = true;
    void            *m_userdata = nullptr;
};

} // namespace usdi
