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
    static void* address(T& s) { return &s; }
};
template<>
struct AttrArgs<TfToken>
{
    static void load(const TfToken& s, void *a, size_t n) { *(const char**)a = s.GetText(); }
    static void store(TfToken& s, const void *a, size_t n) { s = TfToken((const char*)a); }
    static void* address(TfToken& s) { return (void*)s.GetText(); }
};
template<>
struct AttrArgs<std::string>
{
    static void load(const std::string& s, void *a, size_t n) { *(const char**)a = s.c_str(); }
    static void store(std::string& s, const void *a, size_t n) { s = std::string((const char*)a); }
    static void* address(std::string& s) { return (void*)s.c_str(); }
};
template<>
struct AttrArgs<SdfAssetPath>
{
    static void load(const SdfAssetPath& s, void *a, size_t n) { *(const char**)a = s.GetAssetPath().c_str(); }
    static void store(SdfAssetPath& s, const void *a, size_t n) { s = SdfAssetPath((const char*)a); }
    static void* address(SdfAssetPath& s) { return (void*)s.GetAssetPath().c_str(); }
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
    static void* address(VtArray<V>& s) { return (void*)s.cdata(); }
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
    static void* address(VtArray<TfToken>& s) { return (void*)s.cdata(); }
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
    static void* address(VtArray<std::string>& s) { return (void*)s.cdata(); }
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
    static void* address(VtArray<SdfAssetPath>& s) { return (void*)s.cdata(); }
};



Attribute::Attribute(Schema *parent, UsdAttribute usdattr)
    : m_parent(parent), m_usdattr(usdattr)
{
#ifdef usdiDebug
    m_dbg_name = getName();
    m_dbg_typename = getTypeName();
#endif

    if (m_usdattr && m_usdattr.GetNumTimeSamples() > 0) {
        bool dummy;
        m_usdattr.GetBracketingTimeSamples(DBL_MIN, &m_time_start, &m_time_start, &dummy);
        m_usdattr.GetBracketingTimeSamples(DBL_MAX, &m_time_end, &m_time_end, &dummy);
    }
}

Attribute::~Attribute()
{
}

UsdAttribute    Attribute::getUSDAttribute() const  { return m_usdattr; }
Schema*         Attribute::getParent() const        { return m_parent; }
const char*     Attribute::getName() const          { return m_usdattr.GetName().GetText(); }
const char*     Attribute::getTypeName() const      { return m_usdattr.GetTypeName().GetAsToken().GetText(); }
AttributeType   Attribute::getType() const          { return m_type; }
bool            Attribute::isConstant() const       { return !m_usdattr.ValueMightBeTimeVarying(); }
bool            Attribute::hasValue() const         { return m_usdattr.HasValue(); }
size_t          Attribute::getNumSamples() const    { return m_usdattr.GetNumTimeSamples(); }

bool Attribute::getTimeRange(Time& start, Time& end)
{
    if (m_time_start != usdiInvalidTime && m_time_end != usdiInvalidTime) {
        start = m_time_start;
        end = m_time_end;
        return true;
    }
    return false;
}

AttributeSummary Attribute::getSummary()
{
    AttributeSummary ret;
    ret.start = m_time_start;
    ret.end = m_time_end;
    ret.type = m_type;
    ret.num_samples = (int)getNumSamples();
    return ret;
}



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
        m_type = Traits::attr_type;
        usdiLogTrace("Attribute::Attribute(): %s (%s)\n", getName(), getTypeName());
    }

    ~TAttribute()
    {
        usdiLogTrace("Attribute::~Attribute()\n");
    }


    void updateSample(Time t) override
    {
        if (t != m_time_prev) {
            m_time_prev = t;
            m_usdattr.Get(&m_sample, t);
        }
    }

    bool readSample(AttributeData& dst, Time t, bool copy) override
    {
        updateSample(t);

        dst.num_elements = 1;
        if (copy) {
            if (dst.data) {
                Args::load(m_sample, dst.data, 1);
            }
        }
        else {
            dst.data = Args::address(m_sample);
        }
        return true;
    }

    bool writeSample(const AttributeData& src, Time t) override
    {
        Args::store(m_sample, src.data, src.num_elements);
        m_usdattr.Set(m_sample, t);
        return true;
    }

    bool getImmediate(void *dst, Time t) override
    {
        return m_usdattr.Get((T*)dst, t);
    }

    bool setImmediate(const void *src, Time t) override
    {
        return m_usdattr.Set(*(const T*)src, t);
    }

private:
    T m_sample;
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
        m_type = Traits::attr_type;
        usdiLogTrace("Attribute::Attribute(): %s (%s)\n", getName(), getTypeName());
    }

    ~TAttribute()
    {
        usdiLogTrace("Attribute::~Attribute()\n");
    }

    void updateSample(Time t) override
    {
        if (t != m_time_prev) {
            m_time_prev = t;
            m_usdattr.Get(&m_sample, t);
        }
    }

    bool readSample(AttributeData& dst, Time t, bool copy) override
    {
        updateSample(t);

        dst.num_elements = (int)m_sample.size();
        if (copy) {
            if (dst.data) {
                Args::load(m_sample, dst.data, dst.num_elements);
            }
        }
        else {
            dst.data = Args::address(m_sample);
        }
        return true;
    }

    bool writeSample(const AttributeData& src, Time t) override
    {
        Args::store(m_sample, src.data, src.num_elements);
        m_usdattr.Set(m_sample, t);
        return true;
    }

    bool getImmediate(void *dst, Time t) override
    {
        return m_usdattr.Get((VtArray<V>*)dst, t);
    }

    bool setImmediate(const void *src, Time t) override
    {
        return m_usdattr.Set(*(const VtArray<V>*)src, t);
    }

private:
    VtArray<V> m_sample;
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
