#include "pch.h"
#include "usdiInternal.h"
#include "usdiContext.h"
#include "usdiSchema.h"
#include "usdiAttribute.h"

namespace usdi {


// SdfValueTypeNames has no Matrix2f, Matrix3f, Matrix4f.
// because of this, store these as Matrix2d, Matrix3d, Matrix4d :(
#define EachAttributeTypes(Body)\
    Body(bool, AttributeType::Bool, SdfValueTypeNames->Bool, TAttribute)\
    Body(byte, AttributeType::Byte, SdfValueTypeNames->UChar, TAttribute)\
    Body(int, AttributeType::Int, SdfValueTypeNames->Int, TAttribute)\
    Body(uint, AttributeType::UInt, SdfValueTypeNames->UInt, TAttribute)\
    Body(float, AttributeType::Float, SdfValueTypeNames->Float, TAttribute)\
    Body(GfVec2f, AttributeType::Float2, SdfValueTypeNames->Float2, TAttribute)\
    Body(GfVec3f, AttributeType::Float3, SdfValueTypeNames->Float3, TAttribute)\
    Body(GfVec4f, AttributeType::Float4, SdfValueTypeNames->Float4, TAttribute)\
    Body(GfQuatf, AttributeType::Quaternion, SdfValueTypeNames->Quatf, TAttribute)\
    Body(GfMatrix2f, AttributeType::Float2x2, SdfValueTypeNames->Matrix2d, TAttributeWrapped)\
    Body(GfMatrix3f, AttributeType::Float3x3, SdfValueTypeNames->Matrix3d, TAttributeWrapped)\
    Body(GfMatrix4f, AttributeType::Float4x4, SdfValueTypeNames->Matrix4d, TAttributeWrapped)\
    Body(std::string, AttributeType::String, SdfValueTypeNames->String, TAttribute)\
    Body(TfToken, AttributeType::Token, SdfValueTypeNames->Token, TAttribute)\
    Body(SdfAssetPath, AttributeType::Asset, SdfValueTypeNames->Asset, TAttribute)\
    Body(VtArray<bool>, AttributeType::BoolArray, SdfValueTypeNames->BoolArray, TAttribute)\
    Body(VtArray<byte>, AttributeType::ByteArray, SdfValueTypeNames->UCharArray, TAttribute)\
    Body(VtArray<int>, AttributeType::IntArray, SdfValueTypeNames->IntArray, TAttribute)\
    Body(VtArray<uint>, AttributeType::UIntArray, SdfValueTypeNames->UIntArray, TAttribute)\
    Body(VtArray<float>, AttributeType::FloatArray, SdfValueTypeNames->FloatArray, TAttribute)\
    Body(VtArray<GfVec2f>, AttributeType::Float2Array, SdfValueTypeNames->Float2Array, TAttribute)\
    Body(VtArray<GfVec3f>, AttributeType::Float3Array, SdfValueTypeNames->Float3Array, TAttribute)\
    Body(VtArray<GfVec4f>, AttributeType::Float4Array, SdfValueTypeNames->Float4Array, TAttribute)\
    Body(VtArray<GfQuatf>, AttributeType::QuaternionArray, SdfValueTypeNames->QuatfArray, TAttribute)\
    Body(VtArray<GfMatrix2f>, AttributeType::Float2x2Array, SdfValueTypeNames->Matrix2dArray, TAttributeWrapped)\
    Body(VtArray<GfMatrix3f>, AttributeType::Float3x3Array, SdfValueTypeNames->Matrix3dArray, TAttributeWrapped)\
    Body(VtArray<GfMatrix4f>, AttributeType::Float4x4Array, SdfValueTypeNames->Matrix4dArray, TAttributeWrapped)\
    Body(VtArray<std::string>, AttributeType::StringArray, SdfValueTypeNames->StringArray, TAttribute)\
    Body(VtArray<TfToken>, AttributeType::TokenArray, SdfValueTypeNames->TokenArray, TAttribute)\
    Body(VtArray<SdfAssetPath>, AttributeType::AssetArray, SdfValueTypeNames->AssetArray, TAttribute)


template<class T> class TAttribute;
template<class T> class TAttributeWrapped;
template<class T> struct AttrTypeTraits;

#define DefTraits(Type, Enum, Sdf, Template)\
    template<> struct AttrTypeTraits<Type> {\
        using value_type = Type;\
        using attr_type = Template<Type>;\
        static const AttributeType type_enum = Enum;\
        static SdfValueTypeName sdf_typename() { return Sdf; }\
    };

EachAttributeTypes(DefTraits)
#undef DefTraits


// generic load / store
template<class T>
struct AttrArgs
{
    static void load(const T& s, void *a, size_t /*n*/) { *(T*)a = s; }
    static void store(T& s, const void *a, size_t /*n*/) { s = *(const T*)a; }
    static void* address(T& s) { return &s; }
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


// specialization for string types
template<>
struct AttrArgs<TfToken>
{
    static void load(const TfToken& s, void *a, size_t /*n*/) { *(const char**)a = s.GetText(); }
    static void store(TfToken& s, const void *a, size_t /*n*/) { s = TfToken((const char*)a); }
    static void* address(TfToken& s) { return (void*)s.GetText(); }
};
template<>
struct AttrArgs<std::string>
{
    static void load(const std::string& s, void *a, size_t /*n*/) { *(const char**)a = s.c_str(); }
    static void store(std::string& s, const void *a, size_t /*n*/) { s = std::string((const char*)a); }
    static void* address(std::string& s) { return (void*)s.c_str(); }
};
template<>
struct AttrArgs<SdfAssetPath>
{
    static void load(const SdfAssetPath& s, void *a, size_t /*n*/) { *(const char**)a = s.GetAssetPath().c_str(); }
    static void store(SdfAssetPath& s, const void *a, size_t /*n*/) { s = SdfAssetPath((const char*)a); }
    static void* address(SdfAssetPath& s) { return (void*)s.GetAssetPath().c_str(); }
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
    using Traits = AttrTypeTraits<T>;
    using Args = AttrArgs<T>;

    TAttribute(Schema *parent, UsdAttribute usdattr)
        : super(parent, usdattr)
    {
        m_type = Traits::type_enum;
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
        updateSample(t);
        *(T*)dst = m_sample;
        return true;
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
    using T = VtArray<V>;
    using Traits = AttrTypeTraits<T>;
    using Args = AttrArgs<T>;

    TAttribute(Schema *parent, UsdAttribute usdattr)
        : super(parent, usdattr)
    {
        m_type = Traits::type_enum;
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
        updateSample(t);
        *(VtArray<V>*)dst = m_sample;
        return true;
    }

    bool setImmediate(const void *src, Time t) override
    {
        return m_usdattr.Set(*(const VtArray<V>*)src, t);
    }

private:
    VtArray<V> m_sample;
};


template<class T> struct GetStoreType;
template<> struct GetStoreType<GfMatrix2f> { using type = GfMatrix2d; };
template<> struct GetStoreType<GfMatrix3f> { using type = GfMatrix3d; };
template<> struct GetStoreType<GfMatrix4f> { using type = GfMatrix4d; };
template<> struct GetStoreType<VtArray<GfMatrix2f>> { using type = VtArray<GfMatrix2d>; };
template<> struct GetStoreType<VtArray<GfMatrix3f>> { using type = VtArray<GfMatrix3d>; };
template<> struct GetStoreType<VtArray<GfMatrix4f>> { using type = VtArray<GfMatrix4d>; };

template<class T>
class TAttributeWrapped : public Attribute
{
typedef Attribute super;
public:
    using Traits = AttrTypeTraits<T>;
    using StoreT = typename GetStoreType<T>::type;

    TAttributeWrapped(Schema *parent, UsdAttribute usdattr)
        : super(parent, usdattr)
    {
        m_type = Traits::type_enum;
        usdiLogTrace("TAttributeWrapped::TAttributeWrapped(): %s (%s)\n", getName(), getTypeName());
    }

    ~TAttributeWrapped()
    {
        usdiLogTrace("TAttributeWrapped::~TAttributeWrapped()\n");
    }

    void updateSample(Time t) override
    {
        if (t != m_time_prev) {
            m_time_prev = t;
            m_usdattr.Get(&m_tmp, t);
            m_sample = T(m_tmp);
        }
    }

    bool readSample(AttributeData& dst, Time t, bool copy) override
    {
        updateSample(t);

        dst.num_elements = 1;
        if (copy) {
            if (dst.data) {
                *(T*)dst.data = m_sample;
            }
        }
        else {
            dst.data = &m_sample;
        }
        return true;
    }

    bool writeSample(const AttributeData& src, Time t) override
    {
        m_tmp = StoreT(*(const T*)src.data);
        m_usdattr.Set(m_tmp, t);
        return true;
    }

    bool getImmediate(void *dst, Time t) override
    {
        updateSample(t);
        *(T*)dst = m_sample;
        return true;
    }

    bool setImmediate(const void *src, Time t) override
    {
        m_tmp = StoreT(*(T*)src);
        return m_usdattr.Set(m_tmp, t);
    }

private:
    StoreT m_tmp;
    T m_sample;
};


template<class T, class U>
struct TAssigner
{
    void operator()(VtArray<T>& dst, const VtArray<U>& src)
    {
        size_t n = src.size();
        dst.resize(n);
        for (size_t i = 0; i < n; ++i) { dst[i] = T(src[i]); }
    }
    void operator()(T* dst, size_t dst_len, const VtArray<T>& src)
    {
        size_t n = std::min<size_t>(dst_len, src.size());
        for (size_t i = 0; i < n; ++i) { dst[i] = src[i]; }
    }
    void operator()(VtArray<U>& dst, const VtArray<T>& src)
    {
        size_t n = src.size();
        dst.resize(n);
        for (size_t i = 0; i < n; ++i) { dst[i] = U(src[i]); }
    }
    void operator()(VtArray<U>& dst, const T *src, size_t src_len)
    {
        size_t n = src_len;
        dst.resize(n);
        for (size_t i = 0; i < n; ++i) { dst[i] = U(src[i]); }
    }
};

template<class V>
class TAttributeWrapped<VtArray<V>> : public Attribute
{
typedef Attribute super;
public:
    using T = VtArray<V>;
    using Traits = AttrTypeTraits<T>;
    using StoreT = typename GetStoreType<T>::type;
    using Assign = TAssigner<V, typename StoreT::value_type>;

    TAttributeWrapped(Schema *parent, UsdAttribute usdattr)
        : super(parent, usdattr)
    {
        m_type = Traits::type_enum;
        usdiLogTrace("TAttributeWrapped::TAttributeWrapped(): %s (%s)\n", getName(), getTypeName());
    }

    ~TAttributeWrapped()
    {
        usdiLogTrace("TAttributeWrapped::~TAttributeWrapped()\n");
    }


    void updateSample(Time t) override
    {
        if (t != m_time_prev) {
            m_time_prev = t;
            m_usdattr.Get(&m_tmp, t);
            Assign()(m_sample, m_tmp);
        }
    }

    bool readSample(AttributeData& dst, Time t, bool copy) override
    {
        updateSample(t);

        dst.num_elements = 1;
        if (copy) {
            if (dst.data) {
                Assign()((V*)dst.data, (size_t)dst.num_elements, m_sample);
            }
        }
        else {
            dst.data = &m_sample;
        }
        return true;
    }

    bool writeSample(const AttributeData& src, Time t) override
    {
        Assign()(m_tmp, (const V*)src.data, (size_t)src.num_elements);
        m_usdattr.Set(m_tmp, t);
        return true;
    }

    bool getImmediate(void *dst, Time t) override
    {
        updateSample(t);
        *(T*)dst = m_sample;
        return true;
    }

    bool setImmediate(const void *src, Time t) override
    {
        Assign()(m_tmp, *(T*)src);
        return m_usdattr.Set(m_tmp, t);
    }

private:
    StoreT m_tmp;
    T m_sample;
};

Attribute* WrapExistingAttribute(Schema *parent, UsdAttribute usd)
{
    if (!usd) { return nullptr; }

    auto tname = usd.GetTypeName();
#define Def(Type, Enum, Sdf, Template) if(tname == Sdf) { return new Template<Type>(parent, usd); }
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
    UsdAttribute usd = parent->getUsdPrim().GetAttribute(TfToken(name));
    return WrapExistingAttribute(parent, usd);
}

template<class T>
static Attribute* CreateNewAttribute(Schema *parent, const char *name)
{
    UsdAttribute usd = parent->getUsdPrim().CreateAttribute(TfToken(name), AttrTypeTraits<T>::sdf_typename());
    if (!usd) { return nullptr; }
    return new AttrTypeTraits<T>::attr_type(parent, usd);
}

Attribute* CreateNewAttribute(Schema *parent, const char *name, AttributeType type)
{
    switch (type) {
#define Def(Type, Enum, Sdf, Template) case Enum: return CreateNewAttribute<Type>(parent, name);
        EachAttributeTypes(Def)
#undef Def
    }

    usdiLogWarning("failed to create attribute: %s\n", name);
    return nullptr;
}

#define Def(Type, Enum, Sdf, Template) template class Template<Type>;
EachAttributeTypes(Def)
#undef Def


} // namespace usdi
