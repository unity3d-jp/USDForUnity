#include "pch.h"
#include "usdiInternal.h"
#include "usdiContext.h"
#include "usdiSchema.h"
#include "usdiAttribute.h"

namespace usdi {


#define EachAttributeTypes(Body)\
    Body(bool, AttributeType::Bool, SdfValueTypeNames->Bool)\
    Body(byte, AttributeType::Byte, SdfValueTypeNames->UChar)\
    Body(int, AttributeType::Int, SdfValueTypeNames->Int)\
    Body(uint, AttributeType::UInt, SdfValueTypeNames->UInt)\
    Body(float, AttributeType::Float, SdfValueTypeNames->Float)\
    Body(GfVec2f, AttributeType::Float2, SdfValueTypeNames->Float2)\
    Body(GfVec3f, AttributeType::Float3, SdfValueTypeNames->Float3)\
    Body(GfVec4f, AttributeType::Float4, SdfValueTypeNames->Float4)\
    Body(GfQuatf, AttributeType::Quaternion, SdfValueTypeNames->Quatf)\
    Body(TfToken, AttributeType::Token, SdfValueTypeNames->Token)\
    Body(std::string, AttributeType::String, SdfValueTypeNames->String)\
    Body(SdfAssetPath, AttributeType::Asset, SdfValueTypeNames->Asset)\
    Body(VtArray<bool>, AttributeType::BoolArray, SdfValueTypeNames->BoolArray)\
    Body(VtArray<byte>, AttributeType::ByteArray, SdfValueTypeNames->UCharArray)\
    Body(VtArray<int>, AttributeType::IntArray, SdfValueTypeNames->IntArray)\
    Body(VtArray<uint>, AttributeType::UIntArray, SdfValueTypeNames->UIntArray)\
    Body(VtArray<float>, AttributeType::FloatArray, SdfValueTypeNames->FloatArray)\
    Body(VtArray<GfVec2f>, AttributeType::Float2Array, SdfValueTypeNames->Float2Array)\
    Body(VtArray<GfVec3f>, AttributeType::Float3Array, SdfValueTypeNames->Float3Array)\
    Body(VtArray<GfVec4f>, AttributeType::Float4Array, SdfValueTypeNames->Float4Array)\
    Body(VtArray<GfQuatf>, AttributeType::QuaternionArray, SdfValueTypeNames->QuatfArray)\
    Body(VtArray<TfToken>, AttributeType::TokenArray, SdfValueTypeNames->TokenArray)\
    Body(VtArray<std::string>, AttributeType::StringArray, SdfValueTypeNames->StringArray)\
    Body(VtArray<SdfAssetPath>, AttributeType::AssetArray, SdfValueTypeNames->AssetArray)


template<class T> struct AttrTypeTraits;

#define DefTraits(Type, Attr, Sdf)\
    template<> struct AttrTypeTraits<Type> {\
        typedef Type store_type;\
        static const AttributeType attr_type = Attr;\
        static SdfValueTypeName sdf_typename() { return Sdf; }\
    };

EachAttributeTypes(DefTraits)
#undef DefTraits


template<class T>
struct AttrArgs
{
    static void load(const T& s, void *a, size_t n) { *(T*)a = s; }
    static void store(T& s, const void *a, size_t n) { s = *(const T*)a; }
};
template<>
struct AttrArgs<TfToken>
{
    static void load(const TfToken& s, void *a, size_t n) { *(const char**)a = s.GetText(); }
    static void store(TfToken& s, const void *a, size_t n) { s = TfToken((const char*)a); }
};
template<>
struct AttrArgs<std::string>
{
    static void load(const std::string& s, void *a, size_t n) { *(const char**)a = s.c_str(); }
    static void store(std::string& s, const void *a, size_t n) { s = std::string((const char*)a); }
};
template<>
struct AttrArgs<SdfAssetPath>
{
    static void load(const SdfAssetPath& s, void *a, size_t n) { *(const char**)a = s.GetAssetPath().c_str(); }
    static void store(SdfAssetPath& s, const void *a, size_t n) { s = SdfAssetPath((const char*)a); }
};

template<class V>
struct AttrArgs<VtArray<V>>
{
    static void load(const VtArray<V>& s, void *a, size_t n) {
        if (!s.empty()) {
            memcpy(a, &s[0], sizeof(V) * std::min<size_t>(n, s.size()));
        }
    }
    static void store(VtArray<V>& s, const void *a, size_t n) {
        s.assign((V*)a, (V*)a + n);
    }
};
template<>
struct AttrArgs<VtArray<TfToken>>
{
    static void load(const VtArray<TfToken>& s, void *a, size_t n) {
        auto dst = (const char**)a;
        n = std::min<size_t>(n, s.size());
        for (size_t i = 0; i < n; ++i) {
            dst[i] = s[i].GetText();
        }
    }
    static void store(VtArray<TfToken>& s, const void *a, size_t n) {
        auto src = (const char**)a;
        s.resize(n);
        for (size_t i = 0; i < n; ++i) {
            s[i] = TfToken(src[i]);
        }
    }
};
template<>
struct AttrArgs<VtArray<std::string>>
{
    static void load(const VtArray<std::string>& s, void *a, size_t n) {
        auto dst = (const char**)a;
        n = std::min<size_t>(n, s.size());
        for (size_t i = 0; i < n; ++i) {
            dst[i] = s[i].c_str();
        }
    }
    static void store(VtArray<std::string>& s, const void *a, size_t n) {
        auto src = (const char**)a;
        s.resize(n);
        for (size_t i = 0; i < n; ++i) {
            s[i] = std::string(src[i]);
        }
    }
};
template<>
struct AttrArgs<VtArray<SdfAssetPath>>
{
    static void load(const VtArray<SdfAssetPath>& s, void *a, size_t n) {
        auto dst = (const char**)a;
        n = std::min<size_t>(n, s.size());
        for (size_t i = 0; i < n; ++i) {
            dst[i] = s[i].GetAssetPath().c_str();
        }
    }
    static void store(VtArray<SdfAssetPath>& s, const void *a, size_t n) {
        auto src = (const char**)a;
        s.resize(n);
        for (size_t i = 0; i < n; ++i) {
            s[i] = SdfAssetPath(src[i]);
        }
    }
};



Attribute::Attribute(Schema *parent, UsdAttribute usdattr)
    : m_usdattr(usdattr)
{
}

Attribute::~Attribute()
{
}
const char* Attribute::getName() const      { return m_usdattr.GetName().GetText(); }
const char* Attribute::getTypeName() const  { return m_usdattr.GetTypeName().GetAsToken().GetText(); }
bool        Attribute::isArray() const      { return (int)getType() >= (int)AttributeType::UnknownArray; }
bool        Attribute::hasValue() const     { return m_usdattr.HasValue(); }
size_t      Attribute::getNumSamples() const{ return m_usdattr.GetNumTimeSamples(); }


// scalar attribute impl
template<class T>
class TAttribute : public Attribute
{
typedef Attribute super;
public:
    typedef AttrTypeTraits<T> Traits;
    typedef AttrArgs<T> Args;

    TAttribute(Schema *parent, UsdAttribute usdattr)
        : super(parent, usdattr)
    {
        usdiLogTrace("Attribute::Attribute(): %s (%s)\n", getName(), getTypeName());
    }

    ~TAttribute()
    {
        usdiLogTrace("Attribute::~Attribute()\n");
    }

    AttributeType getType() const override { return Traits::attr_type; }
    size_t getArraySize(Time t) const override { return 1; }

    bool get(T& dst, Time t) const { return m_usdattr.Get(&dst, t); }
    bool set(const T& src, Time t) { return m_usdattr.Set(src, t); }

    bool get(void *dst, Time t) const override
    {
        if (!dst) { return false; }
        return get(*(T*)dst, t);
    }

    bool set(const void *src, Time t) override
    {
        if (!src) { return false; }
        return set(*(const T*)src, t);
    }

    bool getBuffered(void *dst, size_t size, Time t) const override
    {
        if (!dst) { return false; }
        get(m_buf, t);
        Args::load(m_buf, dst, size);
        return true;
    }

    bool setBuffered(const void *src, size_t size, Time t) override
    {
        if (!src) { return false; }
        Args::store(m_buf, src, size);
        set(m_buf, t);
        return true;
    }

private:
    mutable T m_buf;
};

// array attribute impl
template<class V>
class TAttribute<VtArray<V>> : public Attribute
{
typedef Attribute super;
public:
    typedef VtArray<V> T;
    typedef AttrTypeTraits<T> Traits;
    typedef AttrArgs<T> Args;

    TAttribute(Schema *parent, UsdAttribute usdattr)
        : super(parent, usdattr)
    {
        usdiLogTrace("Attribute::Attribute(): %s (%s)\n", getName(), getTypeName());
    }

    ~TAttribute()
    {
        usdiLogTrace("Attribute::~Attribute()\n");
    }

    AttributeType getType() const override { return Traits::attr_type; }
    size_t getArraySize(Time t) const override
    {
        if (t != m_prev_time) {
            m_prev_time = t;
            get(m_buf, t);
        }
        return m_buf.size();
    }

    bool get(T& dst, Time t) const { return m_usdattr.Get(&dst, t); }
    bool set(const T& src, Time t) { return m_usdattr.Set(src, t); }

    bool get(void *dst, Time t) const override
    {
        if (!dst) { return false; }
        return get(*(T*)dst, t);
    }

    bool set(const void *src, Time t) override
    {
        if (!src) { return false; }
        return set(*(const T*)src, t);
    }

    bool getBuffered(void *dst, size_t size, Time t) const override
    {
        if (!dst) { return false; }
        if (t != m_prev_time) {
            m_prev_time = t;
            get(m_buf, t);
        }
        Args::load(m_buf, dst, size);
        return true;
    }

    bool setBuffered(const void *src, size_t size, Time t) override
    {
        if (!src) { return false; }
        Args::store(m_buf, src, size);
        set(m_buf, t);
        return true;
    }

private:
    mutable T m_buf;
    mutable Time m_prev_time = DBL_MIN;
};


Attribute* WrapExistingAttribute(Schema *parent, UsdAttribute usd)
{
    if (!usd) { return nullptr; }

    auto tname = usd.GetTypeName();
#define Def(Type, Enum, Sdf) if(tname == Sdf) { return new TAttribute<Type>(parent, usd); }
    EachAttributeTypes(Def)
#undef Def

#define Reinterpret(Sdf, Type) if (tname == SdfValueTypeNames->Sdf) { return new TAttribute<Type>(parent, usd); }
    Reinterpret(Vector3f, GfVec3f)
    Reinterpret(Normal3f, GfVec3f)
    Reinterpret(Point3f, GfVec3f)
    Reinterpret(Color3f, GfVec3f)
    Reinterpret(Vector3fArray, VtArray<GfVec3f>)
    Reinterpret(Normal3fArray, VtArray<GfVec3f>)
    Reinterpret(Point3fArray, VtArray<GfVec3f>)
    Reinterpret(Color3fArray, VtArray<GfVec3f>)
#undef Reinterpret

    usdiLogInfo("failed to interpret attribute: %s (%s)\n", usd.GetName().GetText(), usd.GetTypeName().GetAsToken().GetText());
    return nullptr; // unknown type
}

Attribute* WrapExistingAttribute(Schema *parent, const char *name)
{
    UsdAttribute usd = parent->getUSDPrim().GetAttribute(TfToken(name));
    return WrapExistingAttribute(parent, usd);
}

template<class T>
static Attribute* CreateNewAttribute(Schema *parent, const char *name)
{
    UsdAttribute usd = parent->getUSDPrim().CreateAttribute(TfToken(name), AttrTypeTraits<T>::sdf_typename());
    if (!usd) { return nullptr; }
    return new TAttribute<T>(parent, usd);
}

Attribute* CreateNewAttribute(Schema *parent, const char *name, AttributeType type)
{
    switch (type) {
#define Def(Type, Enum, Sdf) case Enum: return CreateNewAttribute<Type>(parent, name);
        EachAttributeTypes(Def)
#undef Def
    }

    usdiLogWarning("failed to create attribute: %s\n", name);
    return nullptr;
}

#define Def(Type, Enum, Sdf) template class TAttribute<Type>;
EachAttributeTypes(Def)
#undef Def


} // namespace usdi
