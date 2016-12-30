#pragma once

#include "MeshUtils/MeshUtils.h"
#include "etc/RawVector.h"

namespace usdi {

template<class T> T* RawPtr(T *v) { return v; }
template<class T> T* RawPtr(const std::unique_ptr<T>& v) { return v.get(); }

template<class SchemaArray>
inline Schema* FindSchema(SchemaArray& schemas, const char *path)
{
    if (path[0] == '/') {
        for (auto& n : schemas) {
            if (strcmp(n->getPath(), path) == 0) {
                return RawPtr(n);
            }
        }
    }
    else {
        // search node that has specified name
        for (auto& n : schemas) {
            if (strcmp(n->getName(), path) == 0) {
                return RawPtr(n);
            }
        }
    }
    return nullptr;
}


typedef RawVector<char> TempBuffer;

TempBuffer& GetTemporaryBuffer();

} // namespace usdi
