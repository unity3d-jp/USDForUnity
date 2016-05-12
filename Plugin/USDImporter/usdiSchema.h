#pragma once

namespace usdi {

class Schema
{
public:
    Schema(Schema *parent);
    virtual ~Schema();

    Schema*             getParent();
    size_t              getNumChildren();
    Schema*             getChild(int i);

    const char*         getPath();
    const char*         getName();
    UsdPrim             getUSDPrim();
    virtual UsdTyped&   getUSDType() = 0;
    virtual SchemaType  getType() const;

public:
    void setContext(Context *ctx);
    void addChild(Schema *child);

private:
    typedef std::vector<Schema*> Children;
    Context     *m_ctx;
    Schema      *m_parent;
    Children    m_children;
};

} // namespace usdi
