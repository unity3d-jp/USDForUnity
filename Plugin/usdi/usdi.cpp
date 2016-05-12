#include "pch.h"
#include "usdiInternal.h"
#include "usdiSchema.h"
#include "usdiXform.h"
#include "usdiMesh.h"
#include "usdiContext.h"


extern "C" {

usdiExport usdi::Context* usdiOpen(const char *path)
{
    auto *ret = new usdi::Context();
    if (!ret->open(path)) {
        delete ret;
        return nullptr;
    }

    return ret;
}

usdiExport usdi::Context* usdiCreateContext(const char *identifier)
{
    auto *ret = new usdi::Context();
    ret->create(identifier);
    return ret;
}

usdiExport void usdiDestroyContext(usdi::Context *ctx)
{
    delete ctx;
}

usdiExport bool usdiWrite(usdi::Context *ctx, const char *path)
{
    return ctx->write(path);
}

usdiExport void usdiSetImportConfig(usdi::Context *ctx, const usdi::ImportConfig *conf)
{
    ctx->setImportConfig(*conf);
}
usdiExport void usdiGetImportConfig(usdi::Context *ctx, usdi::ImportConfig *conf)
{
    *conf = ctx->getImportConfig();
}
usdiExport void usdiSetExportConfig(usdi::Context *ctx, const usdi::ExportConfig *conf)
{
    ctx->setExportConfig(*conf);
}
usdiExport void usdiGetExportConfig(usdi::Context *ctx, usdi::ExportConfig *conf)
{
    *conf = ctx->getExportConfig();
}

usdiExport usdi::Schema* usdiGetRoot(usdi::Context *ctx)
{
    if (!ctx) return nullptr;

    return ctx->getRootNode();
}


usdiExport int usdiGetID(usdi::Schema *schema)
{
    if (!schema) { return 0; }
    return schema->getID();
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

