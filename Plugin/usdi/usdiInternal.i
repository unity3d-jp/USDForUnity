namespace usdi {

inline bool NearEqual(float a, float b)
{
    const float epsilon = 0.00001f;
    return std::abs(a - b) < epsilon;
}
inline bool NearEqual(const float3& a, const float3& b)
{
    return NearEqual(a.x, b.x) && NearEqual(a.y, b.y) && NearEqual(a.z, b.z);
}
inline bool NearEqual(const quatf& a, const quatf& b)
{
    return NearEqual(a.x, b.x) && NearEqual(a.y, b.y) && NearEqual(a.z, b.z) && NearEqual(a.w, b.w);
}


template<class Int>
inline Int CeilDiv(Int v, Int d)
{
    return v / d + (v % d == 0 ? 0 : 1);
}

inline float2 operator*(const float2& l, float r)
{
    return{ l.x*r, l.y*r };
}

inline float3 operator*(const float3& l, float r)
{
    return{ l.x*r, l.y*r, l.z*r };
}

inline float4 operator*(const float4& l, float r)
{
    return{ l.x*r, l.y*r, l.z*r, l.w*r };
}

inline quatf operator*(const quatf& l, float r)
{
    return{ l.x*r, l.y*r, l.z*r, l.w*r };
}

inline quatf operator*(const quatf& l, const quatf& r)
{
    return{
        l.w*r.x + l.x*r.w + l.y*r.z - l.z*r.y,
        l.w*r.y + l.y*r.w + l.z*r.x - l.x*r.z,
        l.w*r.z + l.z*r.w + l.x*r.y - l.y*r.x,
        l.w*r.w - l.x*r.x - l.y*r.y - l.z*r.z,
    };
}


inline float2& operator*=(float2& l, float r)
{
    l.x *= r;
    l.y *= r;
    return l;
}

inline float3& operator*=(float3& l, float r)
{
    l.x *= r;
    l.y *= r;
    l.z *= r;
    return l;
}

inline float4& operator*=(float4& l, float r)
{
    l.x *= r;
    l.y *= r;
    l.z *= r;
    l.w *= r;
    return l;
}

inline quatf& operator*=(quatf& l, float r)
{
    l.x *= r;
    l.y *= r;
    l.z *= r;
    l.w *= r;
    return l;
}

} // namespace
