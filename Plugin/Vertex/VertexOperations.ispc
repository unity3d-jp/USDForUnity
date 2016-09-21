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
    if(programCount == 4) { // SSE
        const uniform float<4> _c0 = {-1.0f, 1.0f, 1.0f,-1.0f};
        const uniform float<4> _c1 = { 1.0f, 1.0f,-1.0f, 1.0f};
        const uniform float<4> _c2 = { 1.0f,-1.0f, 1.0f, 1.0f};
        const uniform int num_loops = num / 4;

        uniform float<4> * uniform dv = (uniform float<4>* uniform)dst;
        for(uniform int i=0; i < num_loops; ++i) {
            uniform int i3 = i*3;

            prefetch_l1(&dv[i3+3]);

            uniform float<4> _0 = dv[i3 + 0];
            uniform float<4> _1 = dv[i3 + 1];
            uniform float<4> _2 = dv[i3 + 2];
            dv[i3 + 0] = _0 * _c0;
            dv[i3 + 1] = _1 * _c1;
            dv[i3 + 2] = _2 * _c2;
        }
        for(uniform int i=num_loops*4; i < num; ++i) {
            dst[i].x *= -1.0f;
        }
    }
    else if(programCount == 8) { // AVX
        const uniform float<8> _c0 = {-1.0f, 1.0f, 1.0f,-1.0f, 1.0f, 1.0f,-1.0f, 1.0f};
        const uniform float<8> _c1 = { 1.0f,-1.0f, 1.0f, 1.0f,-1.0f, 1.0f, 1.0f,-1.0f};
        const uniform float<8> _c2 = { 1.0f, 1.0f,-1.0f, 1.0f, 1.0f,-1.0f, 1.0f, 1.0f};
        const uniform int num_loops = num / 8;

        uniform float<8> * uniform dv = (uniform float<8>* uniform)dst;
        for(uniform int i=0; i < num_loops; ++i) {
            uniform int i3 = i*3;

            prefetch_l1(&dv[i3+3]);
            prefetch_l1(&dv[i3+5]);

            uniform float<8> _0 = dv[i3 + 0];
            uniform float<8> _1 = dv[i3 + 1];
            uniform float<8> _2 = dv[i3 + 2];
            dv[i3 + 0] = _0 * _c0;
            dv[i3 + 1] = _1 * _c1;
            dv[i3 + 2] = _2 * _c2;
        }
        for(uniform int i=num_loops*8; i < num; ++i) {
            dst[i].x *= -1.0f;
        }
    }
}

// apply scale to every elements
export void ScaleF(uniform float dst[], uniform const float scale, uniform const int num)
{
    foreach(i=0 ... num) {
        dst[i] *= scale;
    }
}


// interleave vertices

export void InterleaveVerticesV3N3(
    uniform vertex_v3n3 dst[],
    uniform const float3 p[],
    uniform const float3 n[],
    uniform const int num)
{

    if(programCount == 4) { // SSE
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
    else if(programCount == 8) { // AVX
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
            dv[5] = _5;
            uniform float<8> _6 = {n2[3], n2[4], p2[5], p2[6], p2[7], n2[5], n2[6], n2[7]};
            dv[6] = _6;
        }
        for(uniform int i=num_loops*8; i < num; ++i) {
            dst[i].p = p[i];
            dst[i].n = n[i];
        }
    }
}

export void InterleaveVerticesV3N3U2(
    uniform vertex_v3n3u2 dst[],
    uniform const float3 p[],
    uniform const float3 n[],
    uniform const float2 u[],
    uniform const int num)
{
    if(programCount == 4) { // SSE
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
    else if(programCount == 8) { // AVX
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
