#include "pch.h"

#include "usdiInternal.h"
#include "usdiAttribute.h"
#include "usdiSchema.h"
#include "usdiXform.h"
#include "usdiCamera.h"
#include "usdiMesh.h"
#include "usdiPoints.h"
#include "usdiContext.h"

#include "usdiExt.h"



extern "C" {

usdiAPI int usdiMemcmp(const void *a, const void *b, int size)
{
    return memcmp(a, b, size);
}
usdiAPI const char* usdiIndexStringArray(const char **v, int i)
{
    if (!v) { return ""; }
    return v[i];
}
usdiAPI void usdiMeshAssignRootBone(usdi::Mesh *mesh, usdi::MeshData *dst, const char *v)
{
    if (!mesh || !dst) { return; }
    mesh->assignRootBone(*dst, v);
}
usdiAPI void usdiMeshAssignBones(usdi::Mesh *mesh, usdi::MeshData *dst, const char **v, int n)
{
    if (!mesh || !dst) { return; }
    mesh->assignBones(*dst, v, n);
}

} // extern "C"
