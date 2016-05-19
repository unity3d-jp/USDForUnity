#pragma once

namespace usdi {

class Schema
{
public:
    Schema(Context *ctx, Schema *parent, const UsdTyped& usd_schema); // for import
    Schema(Context *ctx, Schema *parent, const char *name, const char *type); // for export
    void init();
    virtual ~Schema();

    const ImportConfig& getImportConfig() const;
    const ExportConfig& getExportConfig() const;

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
    UsdPrim             getUSDPrim() const;
    UsdTyped            getUSDSchema() const;
    virtual UsdTyped&   getUSDSchema() = 0;

public:
    void addChild(Schema *child);
    std::string makePath(const char *name);

protected:
    typedef std::vector<Schema*> Children;
    typedef std::unique_ptr<Attribute> AttributePtr;
    typedef std::vector<AttributePtr> Attributes;

    Context     *m_ctx;
    Schema      *m_parent;
    UsdPrim     m_prim;
    Children    m_children;
    Attributes  m_attributes;
    int         m_id = 0;
};

} // namespace usdi
