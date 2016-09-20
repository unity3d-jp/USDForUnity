typedef unsigned int16 half;

struct half2  { half x, y; };
struct half3  { half x, y, z; };
struct half4  { half x, y, z, w; };
struct quath  { half x, y, z, w; };
struct float2 { float x, y; };
struct float3 { float x, y, z; };
struct float4 { float x, y, z, w; };
struct quatf  { float x, y, z, w; };

struct vertex_v3n3
{
    float3 p;
    float3 n;
};

struct vertex_v3n3u2
{
    float3 p;
    float3 n;
    float2 u;
};



float2 half_to_float(half2 h)
{
    float2 ret = {half_to_float(h.x), half_to_float(h.y)};
    return ret;
}
float3 half_to_float(half3 h)
{
    float3 ret = {half_to_float(h.x), half_to_float(h.y), half_to_float(h.z)};
    return ret;
}
float4 half_to_float(half4 h)
{
    float4 ret = {half_to_float(h.x), half_to_float(h.y), half_to_float(h.z), half_to_float(h.w)};
    return ret;
}


export void FloatToHalf(
    uniform half dst[],
    uniform const float src[],
    uniform const int num)
{
    foreach(i=0 ... num) {
        dst[i] = float_to_half(src[i]);
    }
}

export void InvertXF3(uniform float dst[], uniform const int num_)
{
    uniform const int num = num_ * 3;
    if(programCount == 4) {
        uniform float<4> * uniform v = (uniform float<4>* uniform)dst;
        uniform int num_masked = num - (num % 12);
        uniform int num_vectors = num_masked / 4;
        const uniform float<4> _c0 = {-1.0f, 1.0f, 1.0f,-1.0f};
        const uniform float<4> _c1 = { 1.0f, 1.0f,-1.0f, 1.0f};
        const uniform float<4> _c2 = { 1.0f,-1.0f, 1.0f, 1.0f};
        for(uniform int i=0; i < num_vectors; i+=3) {
            uniform float<4> _0 = v[i + 0];
            uniform float<4> _1 = v[i + 1];
            uniform float<4> _2 = v[i + 2];
            v[i + 0] = _0 * _c0;
            v[i + 1] = _1 * _c1;
            v[i + 2] = _2 * _c2;
        }
        for(uniform int i=num_masked; i < num; i+=3) {
            dst[i] *= -1.0f;
        }
    }
    else if(programCount == 8) {
        uniform float<8> * uniform v = (uniform float<8>* uniform)dst;
        uniform int num_masked = num - (num % 24);
        uniform int num_vectors = num_masked / 8;
        const uniform float<8> _c0 = {-1.0f, 1.0f, 1.0f,-1.0f, 1.0f, 1.0f,-1.0f, 1.0f};
        const uniform float<8> _c1 = { 1.0f,-1.0f, 1.0f, 1.0f,-1.0f, 1.0f, 1.0f,-1.0f};
        const uniform float<8> _c2 = { 1.0f, 1.0f,-1.0f, 1.0f, 1.0f,-1.0f, 1.0f, 1.0f};
        for(uniform int i=0; i < num_vectors; i+=3) {
            uniform float<8> _0 = v[i + 0];
            uniform float<8> _1 = v[i + 1];
            uniform float<8> _2 = v[i + 2];
            v[i + 0] = _0 * _c0;
            v[i + 1] = _1 * _c1;
            v[i + 2] = _2 * _c2;
        }
        for(uniform int i=num_masked; i < num; i+=3) {
            dst[i] *= -1.0f;
        }
    }
}

export void ScaleF(uniform float dst[], uniform const float scale, uniform const int num)
{
    foreach(i=0 ... num) {
        dst[i] *= scale;
    }
}


export void InterleaveVerticesV3N3(
    uniform vertex_v3n3 dst[],
    uniform const float3 p[],
    uniform const float3 n[],
    uniform const int num)
{
    for(uniform int i=0; i < num; ++i) {
        dst[i].p = p[i];
        dst[i].n = n[i];
    }
}

export void InterleaveVerticesV3N3U2(
    uniform vertex_v3n3u2 dst[],
    uniform const float3 p[],
    uniform const float3 n[],
    uniform const float2 u[],
    uniform const int num)
{
    if(programCount == 4) {
    }
    else if(programCount == 8) {
        uniform float<8> * uniform pv = (uniform float<8>* uniform)p;
        uniform float<8> * uniform nv = (uniform float<8>* uniform)n;
        uniform float<8> * uniform uv = (uniform float<8>* uniform)u;
        uniform float<8> * uniform dv = (uniform float<8>* uniform)dst;
        uniform int num_vectors = num / 8;
        for(uniform int i=0; i < num_vectors; i+=3) {
            uniform float<8> p0 = pv[i+0], n0 = nv[i+0], u0 = uv[i+0];
            {
                uniform float<8> _0 = {p0[0], p0[1], p0[2], n0[0], n0[1], n0[2], u0[0], u0[1]};
                dv[i*8+0] = _0;
                uniform float<8> _1 = {p0[3], p0[4], p0[5], n0[3], n0[4], n0[5], u0[2], u0[3]};
                dv[i*8+1] = _1;
            }

            uniform float<8> p1 = pv[i+1], n1 = nv[i+1], u1 = uv[i+1];
            {
                uniform float<8> _2 = {p0[6], p0[7], p1[0], n0[6], n0[7], n1[0], u0[4], u0[5]};
                dv[i*8+2] = _2;
                uniform float<8> _3 = {p1[1], p1[2], p1[3], n1[1], n1[2], n1[3], u0[6], u0[7]};
                dv[i*8+3] = _3;
                uniform float<8> _4 = {p1[4], p1[5], p1[6], n1[4], n1[5], n1[6], u1[0], u1[1]};
                dv[i*8+4] = _4;
            }

            uniform float<8> p2 = pv[i+2], n2 = nv[i+2];
            {
                uniform float<8> _5 = {p1[7], p2[0], p2[1], n1[7], n2[0], n2[1], u1[2], u1[3]};
                dv[i*8+5] = _5;
                uniform float<8> _6 = {p2[2], p2[3], p2[4], n2[2], n2[3], n2[4], u1[4], u1[5]};
                dv[i*8+6] = _6;
                uniform float<8> _7 = {p2[5], p2[6], p2[7], n2[5], n2[6], n2[7], u1[6], u1[7]};
                dv[i*8+7] = _7;
            }
        }
        for(uniform int i=num_vectors*8; i < num; ++i) {
            dst[i].p = p[i];
            dst[i].n = n[i];
            dst[i].u = u[i];
        }
    }
}
