namespace usdi {

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

inline quaternion operator*(const quaternion& l, float r)
{
    return{ l.x*r, l.y*r, l.z*r, l.w*r };
}

inline quaternion operator*(const quaternion& l, const quaternion& r)
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

inline quaternion& operator*=(quaternion& l, float r)
{
    l.x *= r;
    l.y *= r;
    l.z *= r;
    l.w *= r;
    return l;
}

} // namespace
