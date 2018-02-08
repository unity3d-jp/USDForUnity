#pragma once

#include "usdi.h"

extern "C" {

// just for C#
usdiAPI int             usdiMemcmp(const void *a, const void *b, int size);
usdiAPI const char*     usdiIndexStringArray(const char **v, int i);
usdiAPI void            usdiMeshAssignRootBone(usdi::Mesh *mesh, usdi::MeshData *dst, const char *v);
usdiAPI void            usdiMeshAssignBones(usdi::Mesh *mesh, usdi::MeshData *dst, const char **v, int n);

} // extern "C"
