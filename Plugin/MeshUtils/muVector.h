#pragma once

#include <cmath>

namespace mu {

struct float2
{
    float x, y;
    float& operator[](int i) { return ((float*)this)[i]; }
    const float& operator[](int i) const { return ((float*)this)[i]; }
};
struct float3
{
    float x, y, z;
    float& operator[](int i) { return ((float*)this)[i]; }
    const float& operator[](int i) const { return ((float*)this)[i]; }
};
struct float4
{
    float x, y, z, w;
    float& operator[](int i) { return ((float*)this)[i]; }
    const float& operator[](int i) const { return ((float*)this)[i]; }
};
struct quatf
{
    float x, y, z, w;
    float& operator[](int i) { return ((float*)this)[i]; }
    const float& operator[](int i) const { return ((float*)this)[i]; }
};

struct float3x3
{
    float3 v[3];
    float3& operator[](int i) { return v[i]; }
    const float3& operator[](int i) const { return v[i]; }
};
struct float4x4 
{
    float4 v[4];
    float4& operator[](int i) { return v[i]; }
    const float4& operator[](int i) const { return v[i]; }
};


inline bool near_equal(float a, float b)
{
    const float epsilon = 0.00001f;
    return std::abs(a - b) < epsilon;
}
inline bool near_equal(const float3& a, const float3& b)
{
    return near_equal(a.x, b.x) && near_equal(a.y, b.y) && near_equal(a.z, b.z);
}
inline bool near_equal(const quatf& a, const quatf& b)
{
    return near_equal(a.x, b.x) && near_equal(a.y, b.y) && near_equal(a.z, b.z) && near_equal(a.w, b.w);
}

template<class Int>
inline Int ceildiv(Int v, Int d)
{
    return v / d + (v % d == 0 ? 0 : 1);
}


inline float2 operator*(const float2& l, float r)
{
    return{ l.x*r, l.y*r };
}

inline float3 operator+(const float3& l, const float3& r)
{
    return{ l.x + r.x, l.y + r.y, l.z + r.z };
}
inline float3 operator-(const float3& l, const float3& r)
{
    return{ l.x - r.x, l.y - r.y, l.z - r.z };
}
inline float3 operator*(const float3& l, float r)
{
    return{ l.x*r, l.y*r, l.z*r };
}
inline float3 operator/(const float3& l, float r)
{
    return{ l.x / r, l.y / r, l.z / r };
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

inline float3& operator+=(float3& l, const float3& r)
{
    l.x += r.x;
    l.y += r.y;
    l.z += r.z;
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

inline float dot(const float3& l, const float3& r)
{
    return l.x*r.x + l.y*r.y + l.z*r.z;
}

inline float3 normalize(const float3& l)
{
    float d = 1.0f / std::sqrt(dot(l, l));
    return l * d;
}

inline float3 cross(const float3& l, const float3& r)
{
    return{
        l.y * r.z - l.z * r.y,
        l.z * r.x - l.x * r.z,
        l.x * r.y - l.y * r.x };
}

inline quatf swap_handedness(const quatf& q)
{
    return { q.x, -q.y, -q.z, q.w };
}

inline float3x3 swap_handedness(const float3x3& m)
{
    return{ {
        { m[0].x, m[0].z, m[0].y },
        { m[2].x, m[2].z, m[2].y },
        { m[1].x, m[1].z, m[1].y },
    } };
}

inline float4x4 swap_handedness(const float4x4& m)
{
    return{ {
        { m[0].x, m[0].z, m[0].y, m[0].w },
        { m[2].x, m[2].z, m[2].y, m[2].w },
        { m[1].x, m[1].z, m[1].y, m[1].w },
        {-m[3].x, m[3].z, m[3].y, m[3].w },
    } };
}

} // namespace mu
