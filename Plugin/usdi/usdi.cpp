#include "pch.h"
#include "usdiInternal.h"
#include "usdiSchema.h"
#include "usdiXform.h"
#include "usdiCamera.h"
#include "usdiMesh.h"
#include "usdiPoints.h"
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

usdiExport usdi::Xform* usdiCreateXform(usdi::Schema *parent, const char *name)
{
    // todo
    return nullptr;
}

usdiExport bool usdiXformReadSample(usdi::Xform *xf, usdi::XformData *dst, usdi::Time t)
{
    if (!xf || !dst) return false;
    return xf->readSample(*dst, t);
}

usdiExport bool usdiXformWriteSample(usdi::Xform *xf, const usdi::XformData *src, usdi::Time t)
{
    if (!xf || !src) return false;
    return xf->writeSample(*src, t);
}


usdiExport usdi::Camera* usdiAsCamera(usdi::Schema *schema)
{
    if (!schema) return nullptr;
    return dynamic_cast<usdi::Camera*>(schema);
}

usdiExport usdi::Camera* usdiCreateCamera(usdi::Schema *parent, const char *name)
{
    // todo
    return nullptr;
}

usdiExport bool usdiCameraReadSample(usdi::Camera *cam, usdi::CameraData *dst, usdi::Time t)
{
    if (!cam || !dst) return false;
    return cam->readSample(*dst, t);
}

usdiExport bool usdiCameraWriteSample(usdi::Camera *cam, const usdi::CameraData *src, usdi::Time t)
{
    if (!cam || !src) return false;
    return cam->writeSample(*src, t);
}


usdiExport usdi::Mesh* usdiAsMesh(usdi::Schema *schema)
{
    if (!schema) return nullptr;
    return dynamic_cast<usdi::Mesh*>(schema);
}

usdiExport usdi::Mesh* usdiCreateMesh(usdi::Schema *parent, const char *name)
{
    // todo
    return nullptr;
}

usdiExport void usdiMeshGetSummary(usdi::Mesh *mesh, usdi::MeshSummary *dst)
{
    if (!mesh || !dst) return;
    mesh->getSummary(*dst);
}

usdiExport bool usdiMeshReadSample(usdi::Mesh *mesh, usdi::MeshData *dst, usdi::Time t)
{
    if (!mesh || !dst) return false;
    return mesh->readSample(*dst, t);
}

usdiExport bool usdiMeshWriteSample(usdi::Mesh *mesh, const usdi::MeshData *src, usdi::Time t)
{
    if (!mesh || !src) return false;
    return mesh->writeSample(*src, t);
}


usdiExport usdi::Points* usdiAsPoints(usdi::Schema *schema)
{
    if (!schema) return nullptr;
    return dynamic_cast<usdi::Points*>(schema);
}

usdiExport usdi::Points* usdiCreatePoints(usdi::Schema *parent, const char *name)
{
    // todo
    return nullptr;
}

usdiExport void usdiPointsGetSummary(usdi::Points *points, usdi::PointsSummary *dst)
{
    if (!points || !dst) return;
    points->getSummary(*dst);
}

usdiExport bool usdiPointsReadSample(usdi::Points *points, usdi::PointsData *dst, usdi::Time t)
{
    if (!points || !dst) return false;
    return points->readSample(*dst, t);
}

usdiExport bool usdiPointsWriteSample(usdi::Points *points, const usdi::PointsData *src, usdi::Time t)
{
    if (!points || !src) return false;
    return points->writeSample(*src, t);
}

} // extern "C"

