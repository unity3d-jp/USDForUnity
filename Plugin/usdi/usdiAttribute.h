#pragma once

namespace usdi {

class Attribute
{
public:
    Attribute(Schema *parent, UsdAttribute usdattr);
    virtual ~Attribute();

    virtual AttributeType getType() const = 0;
    virtual bool get(void *data, Time t) const = 0;
    virtual bool set(const void *data, Time t) = 0;

protected:
    Schema *m_parent;
    UsdAttribute m_usdattr;
};


template<class T>
class TAttribute : public Attribute
{
typedef Attribute super;
public:
    TAttribute(Schema *parent, UsdAttribute usdattr); // wrap existing one
    TAttribute(Schema *parent, const char *name); // create new one
    ~TAttribute() override;

    AttributeType getType() const override;

    bool get(T& dst, Time t) const;
    bool set(const T& dst, Time t);

    bool get(void *data, Time t) const override;
    bool set(const void *data, Time t) override;
};

#define EachAttributeTypeAndEnum(Body)\
    Body(int, AttributeType::Int, SdfValueTypeNames->Int)\
    Body(uint, AttributeType::UInt, SdfValueTypeNames->UInt)\
    Body(float, AttributeType::Float, SdfValueTypeNames->Float)\
    Body(GfVec2f, AttributeType::Float2, SdfValueTypeNames->Float2)\
    Body(GfVec3f, AttributeType::Float3, SdfValueTypeNames->Float3)\
    Body(GfVec4f, AttributeType::Float4, SdfValueTypeNames->Float4)\
    Body(GfQuatf, AttributeType::Quaternion, SdfValueTypeNames->Quatf)\
    Body(VtArray<int>, AttributeType::IntArray, SdfValueTypeNames->IntArray)\
    Body(VtArray<uint>, AttributeType::UIntArray, SdfValueTypeNames->UIntArray)\
    Body(VtArray<float>, AttributeType::FloatArray, SdfValueTypeNames->FloatArray)\
    Body(VtArray<GfVec2f>, AttributeType::Float2Array, SdfValueTypeNames->Float2Array)\
    Body(VtArray<GfVec3f>, AttributeType::Float3Array, SdfValueTypeNames->Float3Array)\
    Body(VtArray<GfVec4f>, AttributeType::Float4Array, SdfValueTypeNames->Float4Array)\
    Body(VtArray<GfQuatf>, AttributeType::QuaternionArray, SdfValueTypeNames->QuatfArray)

} // namespace usdi
