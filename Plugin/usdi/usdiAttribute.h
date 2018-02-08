#pragma once

namespace usdi {

class Attribute
{
public:
    Attribute(Schema *parent, UsdAttribute usdattr);
    virtual ~Attribute();

    UsdAttribute    getUSDAttribute() const;
    Schema*         getParent() const;
    const char*     getName() const;
    const char*     getTypeName() const;
    AttributeType   getType() const;
    bool            isConstant() const;
    bool            hasValue() const;
    size_t          getNumSamples() const;
    void            getTimeRange(Time& start, Time& end);

    AttributeSummary getSummary();
    virtual void    updateSample(Time t) = 0;
    virtual bool    readSample(AttributeData& dst, Time t) = 0;
    virtual bool    writeSample(const AttributeData& src, Time t) = 0;
    virtual bool    getImmediate(void *dst, Time t) = 0;
    virtual bool    setImmediate(const void *src, Time t) = 0;

    Attribute*          findConverter(AttributeType external_type);
    virtual Attribute*  findOrCreateConverter(AttributeType external_type);
    void                addConverter(Attribute *attr); // internal

    // this is very slow. should not be used very often.
    // (consider caching in such cases)
    std::vector<Time> getTimeSamples();

    // Body: [](Time t) -> void
    template<class Body>
    void eachTime(const Body& body)
    {
        auto times = getTimeSamples();
        for (auto& t : times) { body(t); }
    }

protected:
    using AttributePtr = std::unique_ptr<Attribute>;
    using Attributes = std::vector<AttributePtr>;

    Schema *m_parent = nullptr;
    UsdAttribute m_usdattr;
    AttributeType m_type = AttributeType::Unknown;
    Time m_time_start = usdiInvalidTime;
    Time m_time_end = usdiInvalidTime;
    Time m_time_prev = usdiInvalidTime;
    Attributes m_converters;

#ifdef usdiDebug
    const char *m_dbg_name = nullptr;
    const char *m_dbg_typename = nullptr;
#endif
};

Attribute* WrapExistingAttribute(Schema *parent, UsdAttribute usd);
Attribute* WrapExistingAttribute(Schema *parent, const char *name);

template<class T>
Attribute* CreateAttribute(Schema *parent, const char *name);
Attribute* CreateAttribute(Schema *parent, const char *name, AttributeType type);


// custom attribute names
#define usdiUVAttrName              "map1"
#define usdiUVAttrName2             "map2"
#define usdiColorAttrName           "colors"
#define usdiBoneWeightsAttrName     "boneWeights"
#define usdiBoneIndicesAttrName     "boneIndices"
#define usdiBindPosesAttrName       "bindposes"
#define usdiBonesAttrName           "bones"
#define usdiRootBoneAttrName        "rootBone"
#define usdiMaxBoneWeightAttrName   "maxBoneWeights"

} // namespace usdi
