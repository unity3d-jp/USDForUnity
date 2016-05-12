#include "pch.h"
#include "usdiInternal.h"
#include "usdiSchema.h"
#include "usdiXform.h"
#include "usdiMesh.h"
#include "usdiContext.h"


extern "C" {

usdiExport usdi::ImportContext* usdiCreateImportContext(const char *path)
{
    auto* ret = new usdi::ImportContext();
    if (!ret->open(path)) {
        delete ret;
        return nullptr;
    }

    return ret;
}

usdiExport void usdiDestroyImportContext(usdi::ImportContext *ctx)
{
    delete ctx;
}


usdiExport usdi::Schema* usdiGetRoot(usdi::ImportContext *ctx)
{
    if (!ctx) return nullptr;

    return ctx->getRootNode();
}


usdiExport const char* usdiGetPath(usdi::Schema *schema)
{
    if (!schema) { return nullptr; }
    return schema->getPath();
}

usdiExport const char* usdiGetName(usdi::Schema *schema)
{
    if (!schema) { return nullptr; }
    return schema->getName();
}

usdiExport usdi::SchemaType usdiGetType(usdi::Schema *schema)
{
    if (!schema) { return usdi::SchemaType::Unknown; }
    return schema->getType();
}

usdiExport usdi::Schema* usdiGetParent(usdi::Schema *schema)
{
    if (!schema) { return nullptr; }
    return schema->getParent();
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

usdiExport void usdiXformReadSample(usdi::Xform *xf, usdi::XformData *dst, usdi::Time t)
{
    if (!xf || !dst) return;
    xf->readSample(*dst, t);
}

usdiExport void usdiXformWriteSample(usdi::Xform *xf, const usdi::XformData *src, usdi::Time t)
{
    if (!xf || !src) return;
    xf->writeSample(*src, t);
}


usdiExport usdi::Mesh* usdiAsMesh(usdi::Schema *schema)
{
    if (!schema) return nullptr;
    return dynamic_cast<usdi::Mesh*>(schema);
}

usdiExport void usdiMeshReadSample(usdi::Mesh *mesh, usdi::MeshData *dst, usdi::Time t)
{
    if (!mesh || !dst) return;
    mesh->readSample(*dst, t);
}

usdiExport void usdiMeshWriteSample(usdi::Mesh *mesh, const usdi::MeshData *src, usdi::Time t)
{
    if (!mesh || !src) return;
    mesh->writeSample(*src, t);
}

} // extern "C"

