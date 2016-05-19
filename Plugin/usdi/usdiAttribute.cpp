#include "pch.h"
#include "usdiInternal.h"
#include "usdiContext.h"
#include "usdiSchema.h"
#include "usdiAttribute.h"

namespace usdi {

template<class T> struct GetAttributeEnum { static const AttributeType value = AttributeType::Unknown; };
template<AttributeType T> struct GetAttributeType { typedef void type; };
template<class T> SdfValueTypeName GetSdfTypeName() { return SdfValueTypeName(); }

#define Def(Type, Enum, Sdf)\
    template<> struct GetAttributeEnum<Type> { static const AttributeType value = Enum; };\
    template<> struct GetAttributeType<Enum> { typedef Type type; };\
    template<> SdfValueTypeName GetSdfTypeName<Type>() { return Sdf; }
EachAttributeTypeAndEnum(Def)
#undef Def


Attribute::Attribute(Schema *parent, UsdAttribute usdattr)
    : m_usdattr(usdattr)
{
}

Attribute::~Attribute()
{
}
const char* Attribute::getName() const { return m_usdattr.GetName().GetText(); }
const char* Attribute::getTypeName() const { return m_usdattr.GetTypeName().GetAsToken().GetText(); }
bool        Attribute::isArray() const { return (int)getType() >= (int)AttributeType::IntArray; }
bool        Attribute::hasValue() const { return m_usdattr.HasValue(); }

template<class T>
class TAttribute : public Attribute
{
typedef Attribute super;
public:
    TAttribute(Schema *parent, UsdAttribute usdattr)
        : super(parent, usdattr)
    {
        usdiTrace("Attribute::Attribute(): %s [%s]\n", getName(), getTypeName());
    }

    ~TAttribute()
    {
        usdiTrace("Attribute::~Attribute()\n");
    }

    AttributeType getType() const override { return GetAttributeEnum<T>::value; }
    size_t getSize(Time t) const override { return 1; }

    bool get(T& dst, Time t) const { return m_usdattr.Get(&dst, t); }
    bool set(const T& src, Time t) { return m_usdattr.Set(src, t); }

    bool get(void *dst, Time t) const override { return get(*(T*)dst, t); }
    bool set(const void *src, Time t) override { return set(*(const T*)src, t); }

    bool getBuffered(void *dst, size_t size, Time t) const override { return get(dst, t); }
    bool setBuffered(const void *src, size_t size, Time t) override { return set(src, t); }
};

template<class V>
class TAttribute<VtArray<V>> : public Attribute
{
typedef Attribute super;
public:
    typedef VtArray<V> T;

    TAttribute(Schema *parent, UsdAttribute usdattr)
        : super(parent, usdattr)
    {
        usdiTrace("Attribute::Attribute(): %s [%s]\n", getName(), getTypeName());
    }

    ~TAttribute()
    {
        usdiTrace("Attribute::~Attribute()\n");
    }

    AttributeType getType() const override { return GetAttributeEnum<T>::value; }
    size_t getSize(Time t) const override
    {
        get(m_buf, t);
        return m_buf.size();
    }

    bool get(T& dst, Time t) const { return m_usdattr.Get(&dst, t); }
    bool set(const T& src, Time t) { return m_usdattr.Set(src, t); }

    bool get(void *dst, Time t) const override { return get(*(T*)dst, t); }
    bool set(const void *src, Time t) override { return set(*(const T*)src, t); }

    bool getBuffered(void *dst, size_t size, Time t) const override
    {
        if (get(m_buf, t)) {
            memcpy(dst, &m_buf[0], sizeof(V)*size);
            return true;
        }
        return false;
    }
    bool setBuffered(const void *src, size_t size, Time t) override
    {
        m_buf.assign((V*)src, (V*)src + size);
        return set(&m_buf, t);
    }

private:
    mutable T m_buf;
};


Attribute* WrapExistingAttribute(Schema *parent, const char *name)
{
    UsdAttribute usd = parent->getUSDPrim().GetAttribute(TfToken(name));
    if (!usd) { return nullptr; }

    auto tname = usd.GetTypeName();
#define Def(Type, Enum, Sdf) if(tname == Sdf) { return new TAttribute<Type>(parent, usd); }
    EachAttributeTypeAndEnum(Def)
#undef Def

    return nullptr; // unknown type
}

template<class T>
static Attribute* CreateNewAttribute(Schema *parent, const char *name)
{
    UsdAttribute usd = parent->getUSDPrim().CreateAttribute(TfToken(name), GetSdfTypeName<T>());
    if (!usd) { return nullptr; }
    return new TAttribute<T>(parent, usd);
}

Attribute* CreateNewAttribute(Schema *parent, const char *name, AttributeType type)
{
    switch (type) {
#define Def(Type, Enum, Sdf) case Enum: return CreateNewAttribute<Type>(parent, name);
        EachAttributeTypeAndEnum(Def)
#undef Def
    }
    return nullptr;
}

#define Def(Type, Enum, Sdf) template class TAttribute<Type>;
EachAttributeTypeAndEnum(Def)
#undef Def


} // namespace usdi
