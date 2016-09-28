#define C programCount
#define I programIndex

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


// invert every x elements of float3 array
export void InvertXF3(uniform float3 dst[], uniform const int num)
{
    const uniform int num_loops = num / C;

    {
        uniform float _c[3][C];
        _c[0][I] = select((C*0 + I)%3==0, -1.0f, 1.0f);
        _c[1][I] = select((C*1 + I)%3==0, -1.0f, 1.0f);
        _c[2][I] = select((C*2 + I)%3==0, -1.0f, 1.0f);

        uniform float * uniform fv = (uniform float * uniform)dst;
        for(uniform int i=0; i < num_loops; ++i) {
            uniform int i3 = i*3;

            float _0 = fv[C*(i3+0) + I];
            float _1 = fv[C*(i3+1) + I];
            float _2 = fv[C*(i3+2) + I];
            _0 *= _c[0][I];
            _1 *= _c[1][I];
            _2 *= _c[2][I];
            fv[C*(i3+0) + I] = _0;
            fv[C*(i3+1) + I] = _1;
            fv[C*(i3+2) + I] = _2;
        }
    }

    for(uniform int i=num_loops*C; i < num; ++i) {
        dst[i].x *= -1.0f;
    }
}


// apply scale to every elements
export void ScaleF(uniform float dst[], uniform const float scale, uniform const int num)
{
    foreach(i=0 ... num) {
        dst[i] *= scale;
    }
}


export void ComputeBounds(
    uniform const float3 p[],
    uniform const int num,
    uniform float3& o_min, uniform float3& o_max)
{
    if(num == 0) { return; }

    uniform float3 rmin = p[0], rmax = p[0];

    const uniform int num_loops = num / C;
    if(num_loops > 2) {
        const uniform float * uniform fv = (const uniform float * uniform)p;
        uniform float tmin[3][C];
        uniform float tmax[3][C];
        tmin[0][I] = tmax[0][I] = fv[C*0 + I];
        tmin[1][I] = tmax[1][I] = fv[C*1 + I];
        tmin[2][I] = tmax[2][I] = fv[C*2 + I];

        for(uniform int i=1; i < num_loops; ++i) {
            uniform const int i3 = i*3;

            float _0 = fv[C*(i3+0) + I];
            float _1 = fv[C*(i3+1) + I];
            float _2 = fv[C*(i3+2) + I];
            tmin[0][I] = min(tmin[0][I], _0);
            tmin[1][I] = min(tmin[1][I], _1);
            tmin[2][I] = min(tmin[2][I], _2);
            tmax[0][I] = max(tmax[0][I], _0);
            tmax[1][I] = max(tmax[1][I], _1);
            tmax[2][I] = max(tmax[2][I], _2);
        }

        float x,y,z;
        aos_to_soa3((uniform float*)&tmin[0], &x, &y, &z);
        rmin.x = reduce_min(x);
        rmin.y = reduce_min(y);
        rmin.z = reduce_min(z);

        aos_to_soa3((uniform float*)&tmax[0], &x, &y, &z);
        rmax.x = reduce_max(x);
        rmax.y = reduce_max(y);
        rmax.z = reduce_max(z);
    }

    for(uniform int i=num_loops*C; i < num; ++i) {
        uniform float3 t = p[i];
        rmin.x = min(rmin.x, t.x);
        rmin.y = min(rmin.y, t.y);
        rmin.z = min(rmin.z, t.z);
        rmax.x = max(rmax.x, t.x);
        rmax.y = max(rmax.y, t.y);
        rmax.z = max(rmax.z, t.z);
    }

    o_min = rmin;
    o_max = rmax;
}


export void Normalize(
    uniform float3 dst[],
    uniform const int num)
{
    uniform const int num_loops = num / C;

    uniform float * uniform fv = (uniform float * uniform)dst;
    for(uniform int i=0; i < num_loops; ++i) {
        uniform const int i3 = i*3;
        float x,y,z;
        aos_to_soa3((uniform float*)&fv[C*i3], &x, &y, &z);
        float d = rsqrt(x*x + y*y + z*z);
        x *= d;
        y *= d;
        z *= d;
        soa_to_aos3(x, y, z, (uniform float*)&fv[C*i3]);
    }

    for(uniform int i=num_loops*C; i < num; ++i) {
        uniform float3 t = dst[i];
        uniform float d = rsqrt(t.x*t.x + t.y*t.y + t.z*t.z);
        t.x *= d;
        t.y *= d;
        t.z *= d;
        dst[i] = t;
    }
}



// interleave vertices

export void InterleaveV3N3(
    uniform vertex_v3n3 dst[],
    uniform const float3 p[],
    uniform const float3 n[],
    uniform const int num)
{

    if(C == 4) { // SSE
        const uniform float<4> * uniform pv = (uniform float<4>* uniform)p;
        const uniform float<4> * uniform nv = (uniform float<4>* uniform)n;
        const uniform int num_loops = num / 4;

        uniform float<8> * uniform dv = (uniform float<8>* uniform)dst;
        for(uniform int i=0; i < num_loops; ++i, dv+=4) {
            uniform const int i3 = i*3, i2 = i*2;

            prefetch_l1(&pv[i3+3]);
            prefetch_l1(&nv[i3+3]);

            uniform float<4> p0 = pv[i3+0], p1 = pv[i3+1], n0 = nv[i3+0];
            uniform float<8> _0 = {p0[0], p0[1], p0[2], n0[0], n0[1], n0[2], p0[3], p1[0]};
            dv[0] = _0;

            uniform float<4> p2 = pv[i3+1], n1 = nv[i3+1];
            uniform float<8> _1 = {p1[1], n0[3], n1[0], n1[1], p1[2], p1[3], p2[0], n1[2]};
            dv[1] = _1;

            uniform float<4> n2 = nv[i3+1];
            uniform float<8> _2 = {n1[3], n2[0], p2[1], p2[2], p2[3], n2[1], n2[2], n2[3]};
            dv[2] = _2;
        }
        for(uniform int i=num_loops*4; i < num; ++i) {
            dst[i].p = p[i];
            dst[i].n = n[i];
        }
    }
    else if(C == 8) { // AVX
        const uniform float<8> * uniform pv = (uniform float<8>* uniform)p;
        const uniform float<8> * uniform nv = (uniform float<8>* uniform)n;
        const uniform int num_loops = num / 8;

        uniform float<8> * uniform dv = (uniform float<8>* uniform)dst;
        for(uniform int i=0; i < num_loops; ++i, dv+=6) {
            uniform const int i3 = i*3, i2 = i*2;

            prefetch_l1(&pv[i3+3]);
            prefetch_l1(&pv[i3+5]);
            prefetch_l1(&nv[i3+3]);
            prefetch_l1(&nv[i3+5]);

            uniform float<8> p0 = pv[i3+0], n0 = nv[i3+0];
            uniform float<8> _0 = {p0[0], p0[1], p0[2], n0[0], n0[1], n0[2], p0[3], p0[4]};
            dv[0] = _0;

            uniform float<8> p1 = pv[i3+1];
            uniform float<8> _1 = {p0[5], n0[3], n0[4], n0[5], p0[6], p0[7], p1[0], n0[6]};
            dv[1] = _1;

            uniform float<8> n1 = nv[i3+1];
            uniform float<8> _2 = {n0[7], n1[0], p1[1], p1[2], p1[3], n1[1], n1[2], n1[3]};
            dv[2] = _2;

            uniform float<8> p2 = pv[i3+2];
            uniform float<8> _3 = {p1[4], p1[5], p1[6], n1[4], n1[5], n1[6], p1[7], p2[0]};
            dv[3] = _3;

            uniform float<8> n2 = nv[i3+2];
            uniform float<8> _5 = {p2[1], n1[7], n2[0], n2[1], p2[2], p2[3], p2[4], n2[2]};
            dv[4] = _5;
            uniform float<8> _6 = {n2[3], n2[4], p2[5], p2[6], p2[7], n2[5], n2[6], n2[7]};
            dv[5] = _6;
        }
        for(uniform int i=num_loops*8; i < num; ++i) {
            dst[i].p = p[i];
            dst[i].n = n[i];
        }
    }
}

export void InterleaveV3N3U2(
    uniform vertex_v3n3u2 dst[],
    uniform const float3 p[],
    uniform const float3 n[],
    uniform const float2 u[],
    uniform const int num)
{
    if(C == 4) { // SSE
        const uniform float<4> * uniform pv = (uniform float<4>* uniform)p;
        const uniform float<4> * uniform nv = (uniform float<4>* uniform)n;
        const uniform float<4> * uniform uv = (uniform float<4>* uniform)u;
        const uniform int num_loops = num / 4;

        uniform float<8> * uniform dv = (uniform float<8>* uniform)dst;
        for(uniform int i=0; i < num_loops; ++i, dv+=4) {
            uniform const int i3 = i*3, i2 = i*2;

            prefetch_l1(&pv[i3+3]);
            prefetch_l1(&nv[i3+3]);
            prefetch_l1(&uv[i2+2]);

            uniform float<4> p0 = pv[i3+0], n0 = nv[i3+0], u0 = uv[i2+0];
            uniform float<8> _0 = {p0[0], p0[1], p0[2],  n0[0], n0[1], n0[2],  u0[0], u0[1]};
            dv[0] = _0;

            uniform float<4> p1 = pv[i3+1], n1 = nv[i3+1];
            uniform float<8> _1 = {p0[3], p1[0], p1[1],  n0[3], n1[0], n1[1],  u0[2], u0[3]};
            dv[1] = _1;

            uniform float<4> p2 = pv[i3+0], n2 = nv[i3+0], u1 = uv[i2+0];
            uniform float<8> _2 = {p1[2], p1[3], p2[0],  n1[2], n1[3], n2[0],  u1[0], u1[1]};
            dv[2] = _2;
            uniform float<8> _3 = {p2[1], p2[2], p2[3],  n2[1], n2[2], n2[3],  u1[2], u1[3]};
            dv[3] = _3;
        }
        for(uniform int i=num_loops*4; i < num; ++i) {
            dst[i].p = p[i];
            dst[i].n = n[i];
            dst[i].u = u[i];
        }
    }
    else if(C == 8) { // AVX
        const uniform float<8> * uniform pv = (uniform float<8>* uniform)p;
        const uniform float<8> * uniform nv = (uniform float<8>* uniform)n;
        const uniform float<8> * uniform uv = (uniform float<8>* uniform)u;
        const uniform int num_loops = num / 8;

        uniform float<8> * uniform dv = (uniform float<8>* uniform)dst;
        for(uniform int i=0; i < num_loops; ++i, dv+=8) {
            uniform const int i3 = i*3, i2 = i*2;

            prefetch_l1(&pv[i3+3]);
            prefetch_l1(&pv[i3+5]);
            prefetch_l1(&nv[i3+3]);
            prefetch_l1(&nv[i3+5]);
            prefetch_l1(&uv[i2+2]);

            uniform float<8> p0 = pv[i3+0], n0 = nv[i3+0], u0 = uv[i2+0];
            uniform float<8> _0 = {p0[0], p0[1], p0[2], n0[0], n0[1], n0[2], u0[0], u0[1]};
            dv[0] = _0;
            uniform float<8> _1 = {p0[3], p0[4], p0[5], n0[3], n0[4], n0[5], u0[2], u0[3]};
            dv[1] = _1;

            uniform float<8> p1 = pv[i3+1], n1 = nv[i3+1];
            uniform float<8> _2 = {p0[6], p0[7], p1[0], n0[6], n0[7], n1[0], u0[4], u0[5]};
            dv[2] = _2;
            uniform float<8> _3 = {p1[1], p1[2], p1[3], n1[1], n1[2], n1[3], u0[6], u0[7]};
            dv[3] = _3;

            uniform float<8> u1 = uv[i2+1];
            uniform float<8> _4 = {p1[4], p1[5], p1[6], n1[4], n1[5], n1[6], u1[0], u1[1]};
            dv[4] = _4;

            uniform float<8> p2 = pv[i3+2], n2 = nv[i3+2];
            uniform float<8> _5 = {p1[7], p2[0], p2[1], n1[7], n2[0], n2[1], u1[2], u1[3]};
            dv[5] = _5;
            uniform float<8> _6 = {p2[2], p2[3], p2[4], n2[2], n2[3], n2[4], u1[4], u1[5]};
            dv[6] = _6;
            uniform float<8> _7 = {p2[5], p2[6], p2[7], n2[5], n2[6], n2[7], u1[6], u1[7]};
            dv[7] = _7;
        }
        for(uniform int i=num_loops*8; i < num; ++i) {
            dst[i].p = p[i];
            dst[i].n = n[i];
            dst[i].u = u[i];
        }
    }
}
