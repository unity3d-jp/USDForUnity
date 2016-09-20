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
        uniform int num_masked = num - (num % 12);
        for(uniform int i=0; i < num_masked; i+=12) {
            dst[i + 0] *= -1.0f;
            dst[i + 3] *= -1.0f;
            dst[i + 6] *= -1.0f;
            dst[i + 9] *= -1.0f;
        }
        for(uniform int i=num_masked; i < num; i+=3) {
            dst[i] *= -1.0f;
        }
    }
    else if(programCount == 8) {
        uniform int num_masked = num - (num % 24);
        for(uniform int i=0; i < num_masked; i+=24) {
            dst[i + 0] *= -1.0f;
            dst[i + 3] *= -1.0f;
            dst[i + 6] *= -1.0f;
            dst[i + 9] *= -1.0f;
            dst[i +12] *= -1.0f;
            dst[i +15] *= -1.0f;
            dst[i +18] *= -1.0f;
            dst[i +21] *= -1.0f;
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
    foreach(i=0 ... num) {
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
    foreach(i=0 ... num) {
        dst[i].p = p[i];
        dst[i].n = n[i];
        dst[i].u = u[i];
    }
}
