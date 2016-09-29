#pragma once

#include "MeshUtils/MeshUtils.h"
#include "etc/RawVector.h"

namespace usdi {


typedef RawVector<char> TempBuffer;

TempBuffer& GetTemporaryBuffer();


template<class SourceT>
inline void InterleaveBuffered(TempBuffer& buf, const SourceT& src, size_t num)
{
    using vertex_t = SourceT::vertex_t;
    buf.resize(sizeof(vertex_t) * num);
    Interleave((vertex_t*)buf.data(), src, num);
}

} // namespace usdi
