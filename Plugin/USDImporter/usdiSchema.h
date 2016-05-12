#pragma once

namespace usdi {

class Sample
{

};


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

public:
    void addChild(Schema *child);

private:
    Schema *m_parent;
    std::vector<Schema*> m_children;
};

} // namespace usdi
