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
    bool            getTimeRange(Time& start, Time& end);

    AttributeSummary getSummary();
    virtual void    updateSample(Time t) = 0;
    virtual bool    readSample(AttributeData& dst, Time t, bool copy) = 0;
    virtual bool    writeSample(const AttributeData& src, Time t) = 0;
    virtual bool    getImmediate(void *dst, Time t) = 0;
    virtual bool    setImmediate(const void *src, Time t) = 0;

protected:
    Schema *m_parent = nullptr;
    UsdAttribute m_usdattr;
    AttributeType m_type;
    Time m_time_start = usdiInvalidTime, m_time_end = usdiInvalidTime;
    Time m_time_prev = usdiInvalidTime;

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

template<class Internal, class External>
Attribute* CreateWrappedAttribute(Schema *parent, const char *name);
Attribute* CreateWrappedAttribute(Schema *parent, const char *name, AttributeType internal_type, AttributeType external_type);


// custom attribute names
#define usdiUVAttrName              "primvars:uv"
#define usdiUVAttrName2             "uv"
#define usdiTangentAttrName         "tangents"
#define usdiBoneWeightsAttrName     "boneWeights"
#define usdiBoneIndicesAttrName     "boneIndices"
#define usdiBindPosesAttrName       "bindposes"
#define usdiBonesAttrName           "bones"
#define usdiRootBoneAttrName        "rootBone"
#define usdiMaxBoneWeightAttrName   "maxBoneWeights"

} // namespace usdi
