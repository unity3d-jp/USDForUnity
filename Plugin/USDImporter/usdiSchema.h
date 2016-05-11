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

    size_t getNumChildren();
    Schema* getChild(int i);

public:
    void addChild(Schema *child);

private:
    Schema *m_parent;
    std::vector<Schema*> m_children;
};

} // namespace usdi
