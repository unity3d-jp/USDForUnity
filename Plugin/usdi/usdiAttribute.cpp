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
    Body(GfVec2d, AttributeType::Double2, SdfValueTypeNames->Double2, TAttribute<GfVec2d>)\
    Body(GfVec3d, AttributeType::Double3, SdfValueTypeNames->Double3, TAttribute<GfVec3d>)\
    Body(GfVec4d, AttributeType::Double4, SdfValueTypeNames->Double4, TAttribute<GfVec4d>)\
    Body(GfQuatd, AttributeType::QuatD, SdfValueTypeNames->Quatd, TAttribute<GfQuatd>)\
    Body(GfMatrix2d, AttributeType::Double2x2, SdfValueTypeNames->Matrix2d, TAttribute<GfMatrix2d>)\
    Body(GfMatrix3d, AttributeType::Double3x3, SdfValueTypeNames->Matrix3d, TAttribute<GfMatrix3d>)\
    Body(GfMatrix4d, AttributeType::Double4x4, SdfValueTypeNames->Matrix4d, TAttribute<GfMatrix4d>)\
    Body(std::string, AttributeType::String, SdfValueTypeNames->String, TStringAttribute<std::string>)\
    Body(TfToken, AttributeType::Token, SdfValueTypeNames->Token, TStringAttribute<TfToken>)\
    Body(SdfAssetPath, AttributeType::Asset, SdfValueTypeNames->Asset, TStringAttribute<SdfAssetPath>)\
    Body(VtArray<bool>, AttributeType::BoolArray, SdfValueTypeNames->BoolArray, TAttribute<VtArray<bool>>)\
    Body(VtArray<byte>, AttributeType::ByteArray, SdfValueTypeNames->UCharArray, TAttribute<VtArray<byte>>)\
    Body(VtArray<int>, AttributeType::IntArray, SdfValueTypeNames->IntArray, TAttribute<VtArray<int>>)\
    Body(VtArray<uint>, AttributeType::UIntArray, SdfValueTypeNames->UIntArray, TAttribute<VtArray<uint>>)\
    Body(VtArray<half>, AttributeType::HalfArray, SdfValueTypeNames->HalfArray, TAttribute<VtArray<half>>)\
    Body(VtArray<GfVec2h>, AttributeType::Half2Array, SdfValueTypeNames->Half2Array, TAttribute<VtArray<GfVec2h>>)\
    Body(VtArray<GfVec3h>, AttributeType::Half3Array, SdfValueTypeNames->Half3Array, TAttribute<VtArray<GfVec3h>>)\
    Body(VtArray<GfVec4h>, AttributeType::Half4Array, SdfValueTypeNames->Half4Array, TAttribute<VtArray<GfVec4h>>)\
    Body(VtArray<GfQuath>, AttributeType::QuatHArray, SdfValueTypeNames->QuathArray, TAttribute<VtArray<GfQuath>>)\
    Body(VtArray<float>, AttributeType::FloatArray, SdfValueTypeNames->FloatArray, TAttribute<VtArray<float>>)\
    Body(VtArray<GfVec2f>, AttributeType::Float2Array, SdfValueTypeNames->Float2Array, TAttribute<VtArray<GfVec2f>>)\
    Body(VtArray<GfVec3f>, AttributeType::Float3Array, SdfValueTypeNames->Float3Array, TAttribute<VtArray<GfVec3f>>)\
    Body(VtArray<GfVec4f>, AttributeType::Float4Array, SdfValueTypeNames->Float4Array, TAttribute<VtArray<GfVec4f>>)\
    Body(VtArray<GfQuatf>, AttributeType::QuatFArray, SdfValueTypeNames->QuatfArray, TAttribute<VtArray<GfQuatf>>)\
    Body(VtArray<GfVec2d>, AttributeType::Double2Array, SdfValueTypeNames->Double2Array, TAttribute<VtArray<GfVec2d>>)\
    Body(VtArray<GfVec3d>, AttributeType::Double3Array, SdfValueTypeNames->Double3Array, TAttribute<VtArray<GfVec3d>>)\
    Body(VtArray<GfVec4d>, AttributeType::Double4Array, SdfValueTypeNames->Double4Array, TAttribute<VtArray<GfVec4d>>)\
    Body(VtArray<GfQuatd>, AttributeType::QuatDArray, SdfValueTypeNames->QuatdArray, TAttribute<VtArray<GfQuatd>>)\
    Body(VtArray<GfMatrix2d>, AttributeType::Double2x2Array, SdfValueTypeNames->Matrix2dArray, TAttribute<VtArray<GfMatrix2d>>)\
    Body(VtArray<GfMatrix3d>, AttributeType::Double3x3Array, SdfValueTypeNames->Matrix3dArray, TAttribute<VtArray<GfMatrix3d>>)\
    Body(VtArray<GfMatrix4d>, AttributeType::Double4x4Array, SdfValueTypeNames->Matrix4dArray, TAttribute<VtArray<GfMatrix4d>>)\
    Body(VtArray<std::string>, AttributeType::StringArray, SdfValueTypeNames->StringArray, TStringAttribute<VtArray<std::string>>)\
    Body(VtArray<TfToken>, AttributeType::TokenArray, SdfValueTypeNames->TokenArray, TStringAttribute<VtArray<TfToken>>)\
    Body(VtArray<SdfAssetPath>, AttributeType::AssetArray, SdfValueTypeNames->AssetArray, TStringAttribute<VtArray<SdfAssetPath>>)

template<class T> class TAttribute;
template<class T> class TStringAttribute;
template<class T, class InT> class TConverterAttribute;
template<class T> struct AttrTypeTraits;

#define DefTraits(Type, Enum, Sdf, AType)\
    template<> struct AttrTypeTraits<Type> {\
        using attr_type = AType;\
        static const AttributeType type_enum = Enum;\
        static SdfValueTypeName sdf_typename() { return Sdf; }\
    };
EachAttributeTypes(DefTraits)
#undef DefTraits

#define DefTraits(Type, Enum)\
    template<> struct AttrTypeTraits<Type> {\
        static const AttributeType type_enum = Enum;\
    };
DefTraits(GfMatrix2f, AttributeType::Float2x2)
DefTraits(GfMatrix3f, AttributeType::Float3x3)
DefTraits(GfMatrix4f, AttributeType::Float4x4)
DefTraits(VtArray<GfMatrix2f>, AttributeType::Float2x2Array)
DefTraits(VtArray<GfMatrix3f>, AttributeType::Float3x3Array)
DefTraits(VtArray<GfMatrix4f>, AttributeType::Float4x4Array)
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

Attribute* Attribute::findConverter(AttributeType external_type)
{
    if (m_type == external_type) {
        return this;
    }
    for (auto& a : m_converters) {
        if (a->getType() == external_type) {
            return a.get();
        }
    }
    return nullptr;
}

Attribute* Attribute::findOrCreateConverter(AttributeType external_type)
{
    return findConverter(external_type);
}

void Attribute::addConverter(Attribute *attr)
{
    m_converters.push_back(AttributePtr(attr));
}

struct ConverterFactoryBase
{
    using Creator = std::function<Attribute* (Attribute*)>;
    struct Record {
        AttributeType type;
        Creator creator;
    };
    using Records = std::vector<Record>;
};

template<class T>
struct ConverterFactory : public ConverterFactoryBase
{
    static Records& getRecords() {
        static Records s_records;
        return s_records;
    }

    Attribute* operator()(T *attr, AttributeType external_type)
    {
        if (auto *a = attr->findConverter(external_type)) {
            return a;
        }
        for (auto& r : getRecords()) {
            if (r.type == external_type) {
                auto *ret = r.creator(attr);
                attr->addConverter(ret);
                return ret;
            }
        }
        return nullptr;
    }
};


// scalar attribute impl
template<class T>
class TAttribute : public Attribute
{
typedef Attribute super;
public:
    using this_t = TAttribute;
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

    Attribute* findOrCreateConverter(AttributeType external_type) override
    {
        return ConverterFactory<this_t>()(this, external_type);
    }

private:
    rep_t m_sample;
};

// array attribute impl
template<class T>
class TAttribute<VtArray<T>> : public Attribute
{
typedef Attribute super;
public:
    using this_t = TAttribute;
    using rep_t = VtArray<T>;

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

    Attribute* findOrCreateConverter(AttributeType external_type) override
    {
        return ConverterFactory<this_t>()(this, external_type);
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
class TStringAttribute<VtArray<T>> : public Attribute
{
typedef Attribute super;
public:
    using rep_t = VtArray<T>;

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


template<class T, class InT>
class TConverterAttribute : public Attribute
{
typedef Attribute super;
public:
    using internal_t = InT;
    using external_t = T;

    TConverterAttribute(Attribute *src)
        : super(src->getParent(), src->getUSDAttribute())
    {
        m_type = AttrTypeTraits<T>::type_enum;
        usdiLogTrace("TConverterAttribute::TConverterAttribute(): %s (%s)\n", getName(), getTypeName());
    }

    ~TConverterAttribute()
    {
        usdiLogTrace("TConverterAttribute::~TConverterAttribute()\n");
    }

    void updateSample(Time t) override
    {
        if (t == m_time_prev) { return; }
        m_time_prev = t;
        m_usdattr.Get(&m_tmp, t);
        m_sample = T(m_tmp);
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
        m_tmp = internal_t(*(const T*)src.data);
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

template<class T, class InT>
class TConverterAttribute<VtArray<T>, VtArray<InT>> : public Attribute
{
typedef Attribute super;
public:
    using internal_v = InT;
    using external_v = T;
    using internal_t = VtArray<InT>;
    using external_t = VtArray<T>;
    using Assign = TAssigner<external_v, internal_v>;

    TConverterAttribute(Attribute *src)
        : super(src->getParent(), src->getUSDAttribute())
    {
        m_type = AttrTypeTraits<external_t>::type_enum;
        usdiLogTrace("TConverterAttribute::TConverterAttribute(): %s (%s)\n", getName(), getTypeName());
    }

    ~TConverterAttribute()
    {
        usdiLogTrace("TConverterAttribute::~TConverterAttribute()\n");
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
                Assign()((external_v*)dst.data, (size_t)dst.num_elements, m_sample);
            }
        }
        else {
            dst.data = &m_sample;
        }
        return true;
    }

    bool writeSample(const AttributeData& src, Time t) override
    {
        Assign()(m_tmp, (const external_v*)src.data, (size_t)src.num_elements);
        m_usdattr.Set(m_tmp, t);
        return true;
    }

    bool getImmediate(void *dst, Time t) override
    {
        m_usdattr.Get(&m_tmp, t);
        Assign()(*(external_t*)dst, m_tmp);
        return true;
    }

    bool setImmediate(const void *src, Time t) override
    {
        Assign()(m_tmp, *(external_t*)src);
        return m_usdattr.Set(m_tmp, t);
    }

private:
    internal_t m_tmp;
    external_t m_sample;
};

// make attributes convertible
static struct InitConverterFactory
{
    InitConverterFactory()
    {
#define Block(IType, ...)\
        {\
            using itype = IType;\
            auto& srecords = ConverterFactory<TAttribute<itype>>::getRecords();\
            auto& vrecords = ConverterFactory<TAttribute<VtArray<itype>>>::getRecords();\
            __VA_ARGS__\
        }

#define Add(EType)\
        srecords.push_back({ AttrTypeTraits<EType>::type_enum , [](Attribute *attr) { return new TConverterAttribute<EType, itype>(attr); } });\
        vrecords.push_back({ AttrTypeTraits<VtArray<EType>>::type_enum , [](Attribute *attr) { return new TConverterAttribute<VtArray<EType>, VtArray<itype>>(attr); } });

        Block(GfVec2h, Add(GfVec2f) Add(GfVec2d));
        Block(GfVec3h, Add(GfVec3f) Add(GfVec3d));
        Block(GfVec4h, Add(GfVec4f) Add(GfVec4d));

        Block(GfVec2f, Add(GfVec2h) Add(GfVec2d));
        Block(GfVec3f, Add(GfVec3h) Add(GfVec3d));
        Block(GfVec4f, Add(GfVec4h) Add(GfVec4d));

        Block(GfVec2d, Add(GfVec2h) Add(GfVec2f));
        Block(GfVec3d, Add(GfVec3h) Add(GfVec3f));
        Block(GfVec4d, Add(GfVec4h) Add(GfVec4f));

        Block(GfMatrix2d, Add(GfMatrix2f));
        Block(GfMatrix3d, Add(GfMatrix3f));
        Block(GfMatrix4d, Add(GfMatrix4f));

#undef Add
#undef Block
    }
} g_InitConverterFactory;



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
    Reinterpret(Vector3hArray, TAttribute<VtArray<GfVec3h>>)
    Reinterpret(Normal3hArray, TAttribute<VtArray<GfVec3h>>)
    Reinterpret(Point3hArray,  TAttribute<VtArray<GfVec3h>>)
    Reinterpret(Color3hArray,  TAttribute<VtArray<GfVec3h>>)
    Reinterpret(Color4hArray,  TAttribute<VtArray<GfVec4h>>)
    Reinterpret(Vector3fArray, TAttribute<VtArray<GfVec3f>>)
    Reinterpret(Normal3fArray, TAttribute<VtArray<GfVec3f>>)
    Reinterpret(Point3fArray,  TAttribute<VtArray<GfVec3f>>)
    Reinterpret(Color3fArray,  TAttribute<VtArray<GfVec3f>>)
    Reinterpret(Color4fArray,  TAttribute<VtArray<GfVec4f>>)
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


} // namespace usdi
