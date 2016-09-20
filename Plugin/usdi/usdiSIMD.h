#pragma once

#ifdef usdiEnableISPC
    #include "Vertex/VertexOperations.h"
#endif

namespace usdi {

static inline void InvertX(float3 *dst, size_t num)
{
#ifdef usdiEnableISPC
    ispc::InvertXF3((float*)dst, (int)num);
#else
    for (size_t i = 0; i < num; ++i) {
        dst[i].x *= -1.0f;
    }
#endif
}

static inline void Scale(float3 *dst, float s, size_t num)
{
#ifdef usdiEnableISPC
    ispc::ScaleF((float*)dst, s, (int)num*3);
#else
    for (size_t i = 0; i < num; ++i) {
        dst[i] *= s;
    }
#endif
}

} // namespace usdi
