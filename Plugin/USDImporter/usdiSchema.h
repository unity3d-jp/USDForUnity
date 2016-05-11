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

private:
    Schema *m_parent;
};

} // namespace usdi
