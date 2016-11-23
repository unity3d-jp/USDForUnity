#pragma once

namespace usdi {

#define DefSchemaTraits2(Type, Typename)\
    using UsdType = Type;\
    static const char* _getUsdTypeName() { return Typename; };

#define DefSchemaTraits(Type, Typename)\
    DefSchemaTraits2(Type, Typename)\
    static int _getInheritDepth() { return super::_getInheritDepth() + 1; }\


class Schema
{
friend class Context;
public:
    DefSchemaTraits2(UsdSchemaBase, "");
    static int _getInheritDepth() { return 0; }

    Schema(Context *ctx, Schema *parent, Schema *master, const std::string& path, const UsdPrim& p);
    Schema(Context *ctx, Schema *parent, const UsdPrim& p);
    Schema(Context *ctx, Schema *parent, const char *name, const char *type = _getUsdTypeName()); // for export
    void init();
    virtual void setup();
    virtual ~Schema();

    Context*        getContext() const;
    int             getID() const;
    const char*     getPath() const;
    const char*     getName() const;
    const char*     getUsdTypeName() const;
    UsdPrim         getUsdPrim() const;
    void            getTimeRange(Time& start, Time& end) const;

    // attribute interface

    int             getNumAttributes() const;
    Attribute*      getAttribute(int i) const;
    Attribute*      findAttribute(const char *name) const;
    Attribute*      createAttribute(const char *name, AttributeType type);

    // parent & child interface

    Schema*         getParent() const;
    int             getNumChildren() const;
    Schema*         getChild(int i) const;

    // reference & instance interface

    Schema*         getMaster() const;
    int             getNumInstances() const;
    Schema*         getInstance(int i) const;
    bool            isInstance() const;
    bool            isInstanceable() const;
    bool            isMaster() const;
    void            setInstanceable(bool v);
    // asset_path can be null. in this case, local reference is created.
    bool            addReference(const char *asset_path, const char *prim_path);

    // payload interface

    bool            hasPayload() const;
    void            loadPayload();
    void            unloadPayload();
    bool            setPayload(const char *asset_path, const char *prim_path);

    // variant interface

    int             getNumVariantSets() const;
    const char*     getVariantSetName(int iset) const;
    int             getNumVariants(int iset) const;
    const char*     getVariantName(int iset, int ival) const;
    int             getVariantSelection(int iset) const;
    // clear selection if ival is invalid value
    bool            setVariantSelection(int iset, int ival);
    // return -1 if not found
    int             findVariantSet(const char *name) const;
    // return -1 if not found
    int             findVariant(int iset, const char *name) const;
    // return index of created variant set. if variant set with name already exists, return its index.
    int             createVariantSet(const char *name);
    // return index of created variant. if variant with name already exists, return its index.
    int             createVariant(int iset, const char *name);


    UpdateFlags     getUpdateFlags() const;
    UpdateFlags     getUpdateFlagsPrev() const;
    virtual void    updateSample(Time t);

    void            setUserData(void *v);
    void*           getUserData() const;


    // Body: [](Schema *child) -> void
    template<class Body>
    void eachChild(const Body& body)
    {
        for (auto& c : m_children) { body(c); }
    }

    // recursive eachChild
    // Body: [](Schema *child) -> void
    template<class Body>
    void eachChildR(const Body& body)
    {
        eachChild(body);
        eachChild([&](Schema *c) { c->eachChildR(body); });
    }

    // Body: [](Schema *inst) -> void
    template<class Body>
    void eachInstance(const Body& body)
    {
        for (auto& c : m_instances) { body(c); }
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
    void notifyImportConfigChanged();
    void addChild(Schema *child);
    void addInstance(Schema *instance);
    std::string makePath(const char *name);

    const ImportConfig& getImportConfig() const;
    const ExportConfig& getExportConfig() const;

protected:
    using Children = std::vector<Schema*>;
    using Instances = std::vector<Schema*>;
    using AttributePtr = std::unique_ptr<Attribute>;
    using Attributes = std::vector<AttributePtr>;

    struct VariantSet
    {
        std::string name;
        std::vector<std::string> variants;
    };

    void syncAttributes();
    void syncTimeRange();
    void syncVariantSets();

    Context         *m_ctx = nullptr;
    Schema          *m_parent = nullptr;
    Schema          *m_master = nullptr;
    int             m_id = 0;

    std::string     m_path;
    UsdPrim         m_prim;
    Children        m_children;
    Instances       m_instances;
    Attributes      m_attributes;

    std::vector<VariantSet> m_variant_sets;

    Time            m_time_start = usdiInvalidTime, m_time_end = usdiInvalidTime;
    Time            m_time_prev = usdiInvalidTime;
    UpdateFlags     m_update_flag, m_update_flag_prev, m_update_flag_next;
    void            *m_userdata = nullptr;
};


class ISchemaHandler
{
public:
    virtual             ~ISchemaHandler();
    virtual int         getInheritDepth() = 0;
    virtual const char* getUsdTypeName() = 0;
    virtual bool        isCompatible(const UsdPrim& p) = 0;
    virtual Schema*     create(Context *ctx, Schema *parent, const UsdPrim& p) = 0;
};

template<class SchemaType>
class SchemaHandler : public ISchemaHandler
{
public:
    static SchemaHandler& instance() { static SchemaHandler s_inst; return s_inst; }
    int         getInheritDepth() override { return SchemaType::_getInheritDepth(); }
    const char* getUsdTypeName() override { return SchemaType::_getUsdTypeName(); }
    bool        isCompatible(const UsdPrim& p) override { typename SchemaType::UsdType t(p); return t; }
    Schema*     create(Context *ctx, Schema *parent, const UsdPrim& p) override { return new SchemaType(ctx, parent, p); }
};

Schema* CreateSchema(Context *ctx, Schema *parent, const UsdPrim& p);
void RegisterSchemaHandlerImpl(ISchemaHandler& handler);

#define RegisterSchemaHandler(SchemaType)\
    static struct Register##SchemaType { Register##SchemaType() { RegisterSchemaHandlerImpl(SchemaHandler<SchemaType>::instance()); } } g_Register##SchemaType;\
    template SchemaType* Context::createSchema<SchemaType>(Schema *parent, const char *name);

} // namespace usdi
