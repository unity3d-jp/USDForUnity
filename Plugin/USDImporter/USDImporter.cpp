#include "pch.h"
#include "usdiInternal.h"
#include "usdiSchema.h"
#include "usdiXform.h"
#include "usdiPolyMesh.h"
#include "usdiContext.h"


extern "C" {

usdiExport usdi::ImportContext* usdiOpen(const char *path)
{
    auto* ret = new usdi::ImportContext();
    if (!ret->open(path)) {
        delete ret;
        return nullptr;
    }

    return ret;
}


usdiExport usdi::Schema* usdiGetRoot(usdi::ImportContext *ctx)
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



usdiExport usdi::Mesh* usdiAsMesh(usdi::Schema *schema)
{
    if (!schema) return nullptr;
    return dynamic_cast<usdi::Mesh*>(schema);
}

usdiExport usdi::MeshSample* usdiMeshGetSample(usdi::Mesh *mesh, usdi::Time t)
{
    if (!mesh) return nullptr;

}

usdiExport void usdiMeshSampleCopyData(usdi::MeshSample *sample, usdi::MeshData *data)
{
    if (!sample) return;

}

} // extern "C"

