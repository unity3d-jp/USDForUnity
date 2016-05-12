#pragma once

namespace usdi {

class Schema
{
public:
    Schema(Context *ctx, Schema *parent);
    virtual ~Schema();

    const ImportConfig& getImportConfig() const;
    const ExportConfig& getExportConfig() const;

    int                 getID() const;
    Schema*             getParent();
    size_t              getNumChildren();
    Schema*             getChild(int i);

    const char*         getPath();
    const char*         getName();
    UsdPrim             getUSDPrim();
    virtual UsdTyped&   getUSDType() = 0;
    virtual SchemaType  getType() const;

public:
    void setID(int id);
    void addChild(Schema *child);

private:
    typedef std::vector<Schema*> Children;
    Context     *m_ctx;
    Schema      *m_parent;
    Children    m_children;
    int         m_id = 0;
};

} // namespace usdi
