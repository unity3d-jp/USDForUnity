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

template<class T>
TAttribute<T>::TAttribute(Schema *parent, UsdAttribute usdattr)
    : super(parent, usdattr)
{
}

template<class T>
TAttribute<T>::TAttribute(Schema *parent, const char *name)
    : super(parent, parent->getUSDPrim().CreateAttribute(TfToken(name), GetSdfTypeName<T>()))
{
}

template<class T>
TAttribute<T>::~TAttribute()
{
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

#define Def(Type, Enum, Sdf) template class TAttribute<Type>;
EachAttributeTypeAndEnum(Def)
#undef Def


} // namespace usdi
