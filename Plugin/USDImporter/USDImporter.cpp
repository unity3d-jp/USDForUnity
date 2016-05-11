#include "pch.h"
#include "usdiInternal.h"
#include "usdiSchema.h"
#include "usdiXform.h"
#include "usdiPolyMesh.h"
#include "usdiContext.h"


extern "C" {

usdiExport usdi::Context* usdiOpen(const char *path)
{
    auto* ret = new usdi::Context();
    if (!ret->open(path)) {
        delete ret;
        return nullptr;
    }

    return ret;
}


usdiExport usdi::Schema* usdiGetRoot(usdi::Context *ctx)
{
    if (!ctx) return nullptr;

    return ctx->getRootNode();
}

usdiExport int usdiGetNumChildren(usdi::Schema *schema)
{
    if (!schema) return 0;
    return (int)schema->getNumChildren();
}

usdiExport usdi::Schema* usdiGetChild(usdi::Schema *schema, int i)
{
    if (!schema) return nullptr;
    return schema->getChild(i);

}


usdiExport usdi::Xform* usdiAsXform(usdi::Schema *schema)
{
    if (!schema) return nullptr;
    return dynamic_cast<usdi::Xform*>(schema);
}

usdiExport usdi::XformSample* usdiXformGetSample(usdi::Xform *xf, usdi::Time t)
{
    if (!xf) return nullptr;
    return xf->getSample(t);
}

usdiExport void usdiXformGetData(usdi::XformSample *sample, usdi::XformData *data)
{
    if (!sample) return;

}



usdiExport usdi::PolyMesh* usdiAsPolyMesh(usdi::Schema *schema)
{
    if (!schema) return nullptr;
    return dynamic_cast<usdi::PolyMesh*>(schema);
}

usdiExport usdi::PolyMeshSample* usdiPolyMeshGetSample(usdi::PolyMesh *mesh, usdi::Time t)
{
    if (!mesh) return nullptr;

}

usdiExport void usdiPolyMeshSampleCopyData(usdi::PolyMeshSample *sample, usdi::PolyMeshData *data)
{
    if (!sample) return;

}

} // extern "C"

