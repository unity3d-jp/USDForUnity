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

usdiExport void usdiXformReadData(usdi::Xform *xf, usdi::Time t, usdi::XformData *dst)
{
    if (!xf || !dst) return;
    xf->readSample(t, *dst);
}

usdiExport void usdiXformWriteData(usdi::Xform *xf, usdi::Time t, const usdi::XformData *src)
{
    if (!xf || !src) return;
    xf->writeSample(t, *src);
}


usdiExport usdi::Mesh* usdiAsMesh(usdi::Schema *schema)
{
    if (!schema) return nullptr;
    return dynamic_cast<usdi::Mesh*>(schema);
}

usdiExport void usdiMeshReadSample(usdi::Mesh *mesh, usdi::Time t, usdi::MeshData *dst)
{
    if (!mesh || !dst) return;
    mesh->readSample(t, *dst);
}

usdiExport void usdiMeshWriteSample(usdi::Mesh *mesh, usdi::Time t, const usdi::MeshData *src)
{
    if (!mesh || !src) return;
    mesh->writeSample(t, *src);
}

} // extern "C"

