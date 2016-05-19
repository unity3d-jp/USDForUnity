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
bool Attribute::hasValue() const { m_usdattr.HasValue(); }

template<class T>
class TAttribute : public Attribute
{
    typedef Attribute super;
public:
    TAttribute(Schema *parent, UsdAttribute usdattr);
    ~TAttribute() override;

    AttributeType getType() const override;

    bool get(T& dst, Time t) const;
    bool set(const T& dst, Time t);

    bool get(void *data, Time t) const override;
    bool set(const void *data, Time t) override;
};

template<class T>
TAttribute<T>::TAttribute(Schema *parent, UsdAttribute usdattr)
    : super(parent, usdattr)
{
    usdiTrace("Attribute::Attribute(): %s [%s]\n", getName(), getTypeName());
}

template<class T>
TAttribute<T>::~TAttribute()
{
    usdiTrace("Attribute::~Attribute()\n");
}

template<class T>
AttributeType TAttribute<T>::getType() const
{
    return GetAttributeEnum<T>::value;
}

template<class T>
bool TAttribute<T>::get(T& dst, Time t) const
{
    return m_usdattr.Get(&dst, t);
}

template<class T>
bool TAttribute<T>::set(const T& dst, Time t)
{
    return m_usdattr.Set(dst, t);
}

template<class T>
bool TAttribute<T>::get(void *data, Time t) const
{
    return get(*(T*)data, t);
}

template<class T>
bool TAttribute<T>::set(const void *data, Time t)
{
    return set(*(const T*)data, t);
}


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
