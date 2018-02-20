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

} // extern "C"
