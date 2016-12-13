#include "pch.h"
#include "usdiInternal.h"
#include "usdiContext.h"
#include "usdiSchema.h"
#include "usdiAttribute.h"

namespace usdi {

// workaround to use comma in macros...
#define Comma ,

// SdfValueTypeNames has no Matrix2f, Matrix3f, Matrix4f.
// because of this, store these as Matrix2d, Matrix3d, Matrix4d :(
#define EachAttributeTypes(Body)\
    Body(bool, AttributeType::Bool, SdfValueTypeNames->Bool, TAttribute<bool>)\
    Body(byte, AttributeType::Byte, SdfValueTypeNames->UChar, TAttribute<byte>)\
    Body(int, AttributeType::Int, SdfValueTypeNames->Int, TAttribute<int>)\
    Body(uint, AttributeType::UInt, SdfValueTypeNames->UInt, TAttribute<uint>)\
    Body(half, AttributeType::Half, SdfValueTypeNames->Half, TAttribute<half>)\
    Body(GfVec2h, AttributeType::Half2, SdfValueTypeNames->Half2, TAttribute<GfVec2h>)\
    Body(GfVec3h, AttributeType::Half3, SdfValueTypeNames->Half3, TAttribute<GfVec3h>)\
    Body(GfVec4h, AttributeType::Half4, SdfValueTypeNames->Half4, TAttribute<GfVec4h>)\
    Body(GfQuath, AttributeType::QuatH, SdfValueTypeNames->Quath, TAttribute<GfQuath>)\
    Body(float, AttributeType::Float, SdfValueTypeNames->Float, TAttribute<float>)\
    Body(GfVec2f, AttributeType::Float2, SdfValueTypeNames->Float2, TAttribute<GfVec2f>)\
    Body(GfVec3f, AttributeType::Float3, SdfValueTypeNames->Float3, TAttribute<GfVec3f>)\
    Body(GfVec4f, AttributeType::Float4, SdfValueTypeNames->Float4, TAttribute<GfVec4f>)\
    Body(GfQuatf, AttributeType::QuatF, SdfValueTypeNames->Quatf, TAttribute<GfQuatf>)\
    Body(GfMatrix2f, AttributeType::Float2x2, SdfValueTypeNames->Matrix2d, TWrappedAttribute<GfMatrix2d Comma GfMatrix2f>)\
    Body(GfMatrix3f, AttributeType::Float3x3, SdfValueTypeNames->Matrix3d, TWrappedAttribute<GfMatrix3d Comma GfMatrix3f>)\
    Body(GfMatrix4f, AttributeType::Float4x4, SdfValueTypeNames->Matrix4d, TWrappedAttribute<GfMatrix4d Comma GfMatrix4f>)\
    Body(std::string, AttributeType::String, SdfValueTypeNames->String, TStringAttribute<std::string>)\
    Body(TfToken, AttributeType::Token, SdfValueTypeNames->Token, TStringAttribute<TfToken>)\
    Body(SdfAssetPath, AttributeType::Asset, SdfValueTypeNames->Asset, TStringAttribute<SdfAssetPath>)\
    Body(VtArray<bool>, AttributeType::BoolArray, SdfValueTypeNames->BoolArray, TArrayAttribute<bool>)\
    Body(VtArray<byte>, AttributeType::ByteArray, SdfValueTypeNames->UCharArray, TArrayAttribute<byte>)\
    Body(VtArray<int>, AttributeType::IntArray, SdfValueTypeNames->IntArray, TArrayAttribute<int>)\
    Body(VtArray<uint>, AttributeType::UIntArray, SdfValueTypeNames->UIntArray, TArrayAttribute<uint>)\
    Body(VtArray<half>, AttributeType::HalfArray, SdfValueTypeNames->HalfArray, TArrayAttribute<half>)\
    Body(VtArray<GfVec2h>, AttributeType::Half2Array, SdfValueTypeNames->Half2Array, TArrayAttribute<GfVec2h>)\
    Body(VtArray<GfVec3h>, AttributeType::Half3Array, SdfValueTypeNames->Half3Array, TArrayAttribute<GfVec3h>)\
    Body(VtArray<GfVec4h>, AttributeType::Half4Array, SdfValueTypeNames->Half4Array, TArrayAttribute<GfVec4h>)\
    Body(VtArray<GfQuath>, AttributeType::QuatHArray, SdfValueTypeNames->QuathArray, TArrayAttribute<GfQuath>)\
    Body(VtArray<float>, AttributeType::FloatArray, SdfValueTypeNames->FloatArray, TArrayAttribute<float>)\
    Body(VtArray<GfVec2f>, AttributeType::Float2Array, SdfValueTypeNames->Float2Array, TArrayAttribute<GfVec2f>)\
    Body(VtArray<GfVec3f>, AttributeType::Float3Array, SdfValueTypeNames->Float3Array, TArrayAttribute<GfVec3f>)\
    Body(VtArray<GfVec4f>, AttributeType::Float4Array, SdfValueTypeNames->Float4Array, TArrayAttribute<GfVec4f>)\
    Body(VtArray<GfQuatf>, AttributeType::QuatFArray, SdfValueTypeNames->QuatfArray, TArrayAttribute<GfQuatf>)\
    Body(VtArray<GfMatrix2f>, AttributeType::Float2x2Array, SdfValueTypeNames->Matrix2dArray, TWrappedArrayAttribute<GfMatrix2d Comma GfMatrix2f>)\
    Body(VtArray<GfMatrix3f>, AttributeType::Float3x3Array, SdfValueTypeNames->Matrix3dArray, TWrappedArrayAttribute<GfMatrix3d Comma GfMatrix3f>)\
    Body(VtArray<GfMatrix4f>, AttributeType::Float4x4Array, SdfValueTypeNames->Matrix4dArray, TWrappedArrayAttribute<GfMatrix4d Comma GfMatrix4f>)\
    Body(VtArray<std::string>, AttributeType::StringArray, SdfValueTypeNames->StringArray, TStringArrayAttribute<std::string>)\
    Body(VtArray<TfToken>, AttributeType::TokenArray, SdfValueTypeNames->TokenArray, TStringArrayAttribute<TfToken>)\
    Body(VtArray<SdfAssetPath>, AttributeType::AssetArray, SdfValueTypeNames->AssetArray, TStringArrayAttribute<SdfAssetPath>)

template<class T> class TAttribute;
template<class T> class TArrayAttribute;
template<class T> class TStringAttribute;
template<class T> class TStringArrayAttribute;
template<class InT, class ExT> class TWrappedAttribute;
template<class InT, class ExT> class TWrappedArrayAttribute;
template<class T> struct AttrTypeTraits;

#define DefTraits(Type, Enum, Sdf, AType)\
    template<> struct AttrTypeTraits<Type> {\
        using attr_type = AType;\
        static const AttributeType type_enum = Enum;\
        static SdfValueTypeName sdf_typename() { return Sdf; }\
    };

EachAttributeTypes(DefTraits)
#undef DefTraits




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
    using rep_t = T;

    TAttribute(Schema *parent, UsdAttribute usdattr)
        : super(parent, usdattr)
    {
        m_type = AttrTypeTraits<rep_t>::type_enum;
        usdiLogTrace("TAttribute::TAttribute(): %s (%s)\n", getName(), getTypeName());
    }

    ~TAttribute()
    {
        usdiLogTrace("TAttribute::~TAttribute()\n");
    }

    void updateSample(Time t) override
    {
        if (t == m_time_prev) { return; }
        m_time_prev = t;
        m_usdattr.Get(&m_sample, t);
    }

    bool readSample(AttributeData& dst, Time t, bool copy) override
    {
        updateSample(t);

        dst.num_elements = 1;
        if (copy) {
            if (dst.data) {
                *(rep_t*)dst.data = m_sample;
            }
        }
        else {
            dst.data = &m_sample;
        }
        return true;
    }

    bool writeSample(const AttributeData& src, Time t) override
    {
        m_sample = *(const rep_t*)src.data;
        m_usdattr.Set(m_sample, t);
        return true;
    }

    bool getImmediate(void *dst, Time t) override
    {
        return m_usdattr.Get((rep_t*)dst, t);
    }

    bool setImmediate(const void *src, Time t) override
    {
        return m_usdattr.Set(*(const rep_t*)src, t);
    }

private:
    rep_t m_sample;
};

// array attribute impl
template<class T>
class TArrayAttribute : public Attribute
{
typedef Attribute super;
public:
    using rep_t = VtArray<T>;

    TArrayAttribute(Schema *parent, UsdAttribute usdattr)
        : super(parent, usdattr)
    {
        m_type = AttrTypeTraits<rep_t>::type_enum;
        usdiLogTrace("TArrayAttribute::TArrayAttribute(): %s (%s)\n", getName(), getTypeName());
    }

    ~TArrayAttribute()
    {
        usdiLogTrace("TArrayAttribute::~TArrayAttribute()\n");
    }

    void updateSample(Time t) override
    {
        if (t == m_time_prev) { return; }
        m_time_prev = t;
        m_usdattr.Get(&m_sample, t);
    }

    bool readSample(AttributeData& dst, Time t, bool copy) override
    {
        updateSample(t);

        dst.num_elements = (int)m_sample.size();
        if (copy) {
            if (dst.data) {
                size_t n = std::min<size_t>(m_sample.size(), (size_t)dst.num_elements);
                memcpy(dst.data, m_sample.cdata(), sizeof(T)*n);
            }
        }
        else {
            dst.data = m_sample.data();
        }
        return true;
    }

    bool writeSample(const AttributeData& src, Time t) override
    {
        m_sample.resize(src.num_elements);
        memcpy(m_sample.data(), src.data, sizeof(T)*src.num_elements);
        m_usdattr.Set(m_sample, t);
        return true;
    }

    bool getImmediate(void *dst, Time t) override
    {
        return m_usdattr.Get((rep_t*)dst, t);
    }

    bool setImmediate(const void *src, Time t) override
    {
        return m_usdattr.Set(*(const rep_t*)src, t);
    }

private:
    rep_t m_sample;
};


// string attributes impl

template<class T> const char* cstr(const T& v);
template<> const char* cstr(const std::string& v) { return v.c_str(); }
template<> const char* cstr(const TfToken& v) { return v.GetText(); }
template<> const char* cstr(const SdfAssetPath& v) { return v.GetAssetPath().c_str(); }

template<class T>
class TStringAttribute : public Attribute
{
typedef Attribute super;
public:
    using rep_t = T;

    TStringAttribute(Schema *parent, UsdAttribute usdattr)
        : super(parent, usdattr)
    {
        m_type = AttrTypeTraits<rep_t>::type_enum;
        usdiLogTrace("TStringAttribute::TStringAttribute(): %s (%s)\n", getName(), getTypeName());
    }

    ~TStringAttribute()
    {
        usdiLogTrace("TStringAttribute::~TStringAttribute()\n");
    }

    void updateSample(Time t) override
    {
        if (t == m_time_prev) { return; }
        m_time_prev = t;
        m_usdattr.Get(&m_sample, t);
    }

    bool readSample(AttributeData& dst, Time t, bool copy) override
    {
        updateSample(t);

        dst.num_elements = 1;
        *(const char**)dst.data = cstr(m_sample);
        return true;
    }

    bool writeSample(const AttributeData& src, Time t) override
    {
        m_sample = rep_t((const char*)src.data);
        m_usdattr.Set(m_sample, t);
        return true;
    }

    bool getImmediate(void *dst, Time t) override
    {
        return m_usdattr.Get((rep_t*)dst, t);
    }

    bool setImmediate(const void *src, Time t) override
    {
        return m_usdattr.Set(*(const rep_t*)src, t);
    }

private:
    rep_t m_sample;
};

template<class T>
class TStringArrayAttribute : public Attribute
{
typedef Attribute super;
public:
    using rep_t = VtArray<T>;

    TStringArrayAttribute(Schema *parent, UsdAttribute usdattr)
        : super(parent, usdattr)
    {
        m_type = AttrTypeTraits<rep_t>::type_enum;
        usdiLogTrace("TStringArrayAttribute::TStringArrayAttribute(): %s (%s)\n", getName(), getTypeName());
    }

    ~TStringArrayAttribute()
    {
        usdiLogTrace("TStringArrayAttribute::~TStringArrayAttribute()\n");
    }

    void updateSample(Time t) override
    {
        if (t == m_time_prev) { return; }
        m_time_prev = t;
        m_usdattr.Get(&m_sample, t);

        m_pointers.resize(m_sample.size());
        for (size_t i = 0; i < m_sample.size(); ++i) {
            m_pointers[i] = cstr(m_sample[i]);
        }
        m_pointers.push_back(nullptr);
    }

    bool readSample(AttributeData& dst, Time t, bool copy) override
    {
        updateSample(t);

        dst.num_elements = (int)m_sample.size();
        *(const char***)dst.data = m_pointers.data();
        return true;
    }

    bool writeSample(const AttributeData& src, Time t) override
    {
        m_sample.resize(src.num_elements);
        for (int i = 0; i < src.num_elements; ++i) {
            m_sample[i] = T(((const char**)src.data)[i]);
        }
        m_usdattr.Set(m_sample, t);
        return true;
    }

    bool getImmediate(void *dst, Time t) override
    {
        return m_usdattr.Get((rep_t*)dst, t);
    }

    bool setImmediate(const void *src, Time t) override
    {
        return m_usdattr.Set(*(const rep_t*)src, t);
    }

private:
    rep_t m_sample;
    std::vector<const char*> m_pointers;
};


template<class InT, class ExT>
class TWrappedAttribute : public Attribute
{
typedef Attribute super;
public:
    using internal_t = InT;
    using external_t = ExT;

    TWrappedAttribute(Schema *parent, UsdAttribute usdattr)
        : super(parent, usdattr)
    {
        m_type = AttrTypeTraits<external_t>::type_enum;
        usdiLogTrace("TAttributeWrapped::TAttributeWrapped(): %s (%s)\n", getName(), getTypeName());
    }

    ~TWrappedAttribute()
    {
        usdiLogTrace("TAttributeWrapped::~TAttributeWrapped()\n");
    }

    void updateSample(Time t) override
    {
        if (t == m_time_prev) { return; }
        m_time_prev = t;
        m_usdattr.Get(&m_tmp, t);
        m_sample = external_t(m_tmp);
    }

    bool readSample(AttributeData& dst, Time t, bool copy) override
    {
        updateSample(t);

        dst.num_elements = 1;
        if (copy) {
            if (dst.data) {
                *(external_t*)dst.data = m_sample;
            }
        }
        else {
            dst.data = &m_sample;
        }
        return true;
    }

    bool writeSample(const AttributeData& src, Time t) override
    {
        m_tmp = internal_t(*(const external_t*)src.data);
        m_usdattr.Set(m_tmp, t);
        return true;
    }

    bool getImmediate(void *dst, Time t) override
    {
        m_usdattr.Get(&m_tmp, t);
        *(external_t*)dst = external_t(m_tmp);
        return true;
    }

    bool setImmediate(const void *src, Time t) override
    {
        m_tmp = internal_t(*(external_t*)src);
        return m_usdattr.Set(m_tmp, t);
    }

private:
    internal_t m_tmp;
    external_t m_sample;
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

template<class InT, class ExT>
class TWrappedArrayAttribute : public Attribute
{
typedef Attribute super;
public:
    using internal_t = InT;
    using external_t = ExT;
    using internal_array = VtArray<internal_t>;
    using external_array = VtArray<external_t>;

    using Assign = TAssigner<external_t, internal_t>;

    TWrappedArrayAttribute(Schema *parent, UsdAttribute usdattr)
        : super(parent, usdattr)
    {
        m_type = AttrTypeTraits<external_array>::type_enum;
        usdiLogTrace("TWrappedArrayAttribute::TWrappedArrayAttribute(): %s (%s)\n", getName(), getTypeName());
    }

    ~TWrappedArrayAttribute()
    {
        usdiLogTrace("TWrappedArrayAttribute::~TWrappedArrayAttribute()\n");
    }


    void updateSample(Time t) override
    {
        if (t == m_time_prev) { return; }
        m_time_prev = t;
        m_usdattr.Get(&m_tmp, t);
        Assign()(m_sample, m_tmp);
    }

    bool readSample(AttributeData& dst, Time t, bool copy) override
    {
        updateSample(t);

        dst.num_elements = (int)m_sample.size();
        if (copy) {
            if (dst.data) {
                Assign()((external_t*)dst.data, (size_t)dst.num_elements, m_sample);
            }
        }
        else {
            dst.data = &m_sample;
        }
        return true;
    }

    bool writeSample(const AttributeData& src, Time t) override
    {
        Assign()(m_tmp, (const external_t*)src.data, (size_t)src.num_elements);
        m_usdattr.Set(m_tmp, t);
        return true;
    }

    bool getImmediate(void *dst, Time t) override
    {
        m_usdattr.Get(&m_tmp, t);
        Assign()(*(external_array*)dst, m_tmp);
        return true;
    }

    bool setImmediate(const void *src, Time t) override
    {
        Assign()(m_tmp, *(external_array*)src);
        return m_usdattr.Set(m_tmp, t);
    }

private:
    internal_array m_tmp;
    external_array m_sample;
};

Attribute* WrapExistingAttribute(Schema *parent, UsdAttribute usd)
{
    if (!usd) { return nullptr; }

    auto tname = usd.GetTypeName();
#define Def(Type, Enum, Sdf, AType) if(tname == Sdf) { return new AType(parent, usd); }
    EachAttributeTypes(Def)
#undef Def

#define Reinterpret(Sdf, AttrType) if (tname == SdfValueTypeNames->Sdf) { return new AttrType(parent, usd); }
    Reinterpret(Vector3h, TAttribute<GfVec3h>)
    Reinterpret(Normal3h, TAttribute<GfVec3h>)
    Reinterpret(Point3h,  TAttribute<GfVec3h>)
    Reinterpret(Color3h,  TAttribute<GfVec3h>)
    Reinterpret(Color4h,  TAttribute<GfVec4h>)
    Reinterpret(Vector3f, TAttribute<GfVec3f>)
    Reinterpret(Normal3f, TAttribute<GfVec3f>)
    Reinterpret(Point3f,  TAttribute<GfVec3f>)
    Reinterpret(Color3f,  TAttribute<GfVec3f>)
    Reinterpret(Color4f,  TAttribute<GfVec4f>)
    Reinterpret(Vector3hArray, TArrayAttribute<GfVec3h>)
    Reinterpret(Normal3hArray, TArrayAttribute<GfVec3h>)
    Reinterpret(Point3hArray,  TArrayAttribute<GfVec3h>)
    Reinterpret(Color3hArray,  TArrayAttribute<GfVec3h>)
    Reinterpret(Color4hArray,  TArrayAttribute<GfVec4h>)
    Reinterpret(Vector3fArray, TArrayAttribute<GfVec3f>)
    Reinterpret(Normal3fArray, TArrayAttribute<GfVec3f>)
    Reinterpret(Point3fArray,  TArrayAttribute<GfVec3f>)
    Reinterpret(Color3fArray,  TArrayAttribute<GfVec3f>)
    Reinterpret(Color4fArray,  TArrayAttribute<GfVec4f>)
#undef Reinterpret

    usdiLogInfo("failed to interpret attribute: %s (%s)\n", usd.GetName().GetText(), usd.GetTypeName().GetAsToken().GetText());
    return nullptr; // unknown type
}

Attribute* WrapExistingAttribute(Schema *parent, const char *name)
{
    UsdAttribute usd = parent->getUsdPrim().GetAttribute(TfToken(name));
    if (!usd) { return nullptr; }
    return WrapExistingAttribute(parent, usd);
}

template<class T>
Attribute* CreateAttribute(Schema *parent, const char *name)
{
    UsdAttribute usd = parent->getUsdPrim().CreateAttribute(TfToken(name), AttrTypeTraits<T>::sdf_typename());
    if (!usd) { return nullptr; }
    return new AttrTypeTraits<T>::attr_type(parent, usd);
}

Attribute* CreateAttribute(Schema *parent, const char *name, AttributeType type)
{
    switch (type) {
#define Def(Type, Enum, Sdf, AType) case Enum: return CreateAttribute<Type>(parent, name);
        EachAttributeTypes(Def)
#undef Def
    }

    usdiLogWarning("failed to create attribute: %s\n", name);
    return nullptr;
}

#define Def(Type, Enum, Sdf, AType) template class AType;
EachAttributeTypes(Def)
#undef Def


// wrapped attribute

template<class Internal, class External>
struct WrappedAttributeFactory
{
    static Attribute* create(Schema *parent, UsdAttribute usd)
    {
        return new TWrappedAttribute<Internal, External>(parent, usd);
    }
};
template<class Internal, class External>
struct WrappedAttributeFactory<VtArray<Internal>, VtArray<External>>
{
    static Attribute* create(Schema *parent, UsdAttribute usd)
    {
        return new TWrappedArrayAttribute<Internal, External>(parent, usd);
    }
};

template<class Internal, class External>
Attribute* CreateWrappedAttribute(Schema *parent, const char *name)
{
    UsdAttribute usd = parent->getUsdPrim().CreateAttribute(TfToken(name), AttrTypeTraits<Internal>::sdf_typename());
    if (!usd) { return nullptr; }
    return WrappedAttributeFactory::create(parent, usd);
}


} // namespace usdi
