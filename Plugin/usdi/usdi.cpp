#include "pch.h"
#include "usdiInternal.h"
#include "usdiAttribute.h"
#include "usdiSchema.h"
#include "usdiXform.h"
#include "usdiCamera.h"
#include "usdiMesh.h"
#include "usdiPoints.h"
#include "usdiContext.h"


#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "half.lib")
#pragma comment(lib, "double-conversion.lib")

#pragma comment(lib, "ar.lib")
#pragma comment(lib, "arch.lib")
#pragma comment(lib, "gf.lib")
#pragma comment(lib, "js.lib")
#pragma comment(lib, "kind.lib")
#pragma comment(lib, "pcp.lib")
#pragma comment(lib, "plug.lib")
#pragma comment(lib, "sdf.lib")
#pragma comment(lib, "tf.lib")
#pragma comment(lib, "tracelite.lib")
#pragma comment(lib, "usd.lib")
#pragma comment(lib, "usdGeom.lib")
#pragma comment(lib, "vt.lib")
#pragma comment(lib, "work.lib")

namespace usdi {
extern int g_debug_level;
tbb::task_group g_read_tasks;
tbb::task_group g_write_tasks;
} // namespace usdi

extern "C" {

usdiAPI void usdiSetDebugLevel(int l)
{
    usdi::g_debug_level = l;
}

usdiAPI usdi::Time usdiGetDefaultTime()
{
    return std::numeric_limits<double>::quiet_NaN();
}

usdiAPI void usdiWaitAsyncRead()
{
    usdi::g_read_tasks.wait();
}
usdiAPI void usdiWaitAsyncWrite()
{
    usdi::g_write_tasks.wait();
}



usdiAPI usdi::Context* usdiCreateContext()
{
    usdiTraceFunc();
    return new usdi::Context();
}

usdiAPI void usdiDestroyContext(usdi::Context *ctx)
{
    usdiTraceFunc();
    delete ctx;
}

usdiAPI bool usdiOpen(usdi::Context *ctx, const char *path)
{
    usdiTraceFunc();
    if (!ctx || !path) return false;
    return ctx->open(path);
}

usdiAPI bool usdiCreateStage(usdi::Context *ctx, const char *path)
{
    usdiTraceFunc();
    if (!ctx) return false;
    return ctx->createStage(path);
}

usdiAPI bool usdiSave(usdi::Context *ctx)
{
    usdiTraceFunc();
    if (!ctx) return false;
    return ctx->save();
}

usdiAPI bool usdiWrite(usdi::Context *ctx, const char *path)
{
    usdiTraceFunc();
    if (!ctx || !path) return false;
    return ctx->write(path);
}

usdiAPI void usdiSetImportConfig(usdi::Context *ctx, const usdi::ImportConfig *conf)
{
    usdiTraceFunc();
    if (!ctx || !conf) return;
    ctx->setImportConfig(*conf);
}
usdiAPI void usdiGetImportConfig(usdi::Context *ctx, usdi::ImportConfig *conf)
{
    usdiTraceFunc();
    if (!ctx || !conf) return;
    *conf = ctx->getImportConfig();
}
usdiAPI void usdiSetExportConfig(usdi::Context *ctx, const usdi::ExportConfig *conf)
{
    usdiTraceFunc();
    if (!ctx || !conf) return;
    ctx->setExportConfig(*conf);
}
usdiAPI void usdiGetExportConfig(usdi::Context *ctx, usdi::ExportConfig *conf)
{
    usdiTraceFunc();
    if (!ctx || !conf) return;
    *conf = ctx->getExportConfig();
}

usdiAPI usdi::Schema* usdiGetRoot(usdi::Context *ctx)
{
    usdiTraceFunc();
    if (!ctx) return nullptr;

    return ctx->getRootNode();
}


usdiAPI int usdiGetID(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) { return 0; }
    return schema->getID();
}

usdiAPI const char* usdiGetPath(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) { return ""; }
    return schema->getPath();
}

usdiAPI const char* usdiGetName(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) { return ""; }
    return schema->getName();
}

usdiAPI const char* usdiGetTypeName(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) { return ""; }
    return schema->getTypeName();
}

usdiAPI usdi::Schema* usdiGetParent(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) { return nullptr; }
    return schema->getParent();
}

usdiAPI int usdiGetNumChildren(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) return 0;
    return (int)schema->getNumChildren();
}

usdiAPI usdi::Schema* usdiGetChild(usdi::Schema *schema, int i)
{
    usdiTraceFunc();
    if (!schema) return nullptr;
    return schema->getChild(i);

}
usdiAPI int usdiGetNumAttributes(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) { return 0; }
    return schema->getNumAttributes();
}

usdiAPI usdi::Attribute* usdiGetAttribute(usdi::Schema *schema, int i)
{
    usdiTraceFunc();
    if (!schema) { return nullptr; }
    return schema->getAttribute(i);
}

usdiAPI usdi::Attribute* usdiFindAttribute(usdi::Schema *schema, const char *name)
{
    usdiTraceFunc();
    if (!schema) { return nullptr; }
    return schema->findAttribute(name);
}

usdiAPI usdi::Attribute* usdiCreateAttribute(usdi::Schema *schema, const char *name, usdi::AttributeType type)
{
    usdiTraceFunc();
    if (!schema) { return nullptr; }
    return schema->createAttribute(name, type);
}


usdiAPI usdi::Xform* usdiAsXform(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) return nullptr;
    return dynamic_cast<usdi::Xform*>(schema);
}

usdiAPI usdi::Xform* usdiCreateXform(usdi::Context *ctx, usdi::Schema *parent, const char *name)
{
    usdiTraceFunc();
    if (!ctx) { usdiLogWarning("usdiCreateCamera(): ctx must be set\n"); return nullptr; }
    if (!name) { usdiLogWarning("usdiCreateXform(): name must be set\n"); return nullptr; }
    return new usdi::Xform(ctx, parent, name);
}

usdiAPI bool usdiXformReadSample(usdi::Xform *xf, usdi::XformData *dst, usdi::Time t)
{
    usdiTraceFunc();
    if (!xf || !dst) return false;
    return xf->readSample(*dst, t);
}

usdiAPI bool usdiXformWriteSample(usdi::Xform *xf, const usdi::XformData *src, usdi::Time t)
{
    usdiTraceFunc();
    if (!xf || !src) return false;
    return xf->writeSample(*src, t);
}


usdiAPI usdi::Camera* usdiAsCamera(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) return nullptr;
    return dynamic_cast<usdi::Camera*>(schema);
}

usdiAPI usdi::Camera* usdiCreateCamera(usdi::Context *ctx, usdi::Schema *parent, const char *name)
{
    usdiTraceFunc();
    if (!ctx) { usdiLogWarning("usdiCreateCamera(): ctx must be set\n"); return nullptr; }
    if (!name) { usdiLogWarning("usdiCreateCamera(): name must be set\n"); return nullptr; }
    return new usdi::Camera(ctx, parent, name);
}

usdiAPI bool usdiCameraReadSample(usdi::Camera *cam, usdi::CameraData *dst, usdi::Time t)
{
    usdiTraceFunc();
    if (!cam || !dst) return false;
    return cam->readSample(*dst, t);
}

usdiAPI bool usdiCameraWriteSample(usdi::Camera *cam, const usdi::CameraData *src, usdi::Time t)
{
    usdiTraceFunc();
    if (!cam || !src) return false;
    return cam->writeSample(*src, t);
}


usdiAPI usdi::Mesh* usdiAsMesh(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) return nullptr;
    return dynamic_cast<usdi::Mesh*>(schema);
}

usdiAPI usdi::Mesh* usdiCreateMesh(usdi::Context *ctx, usdi::Schema *parent, const char *name)
{
    usdiTraceFunc();
    if (!ctx) { usdiLogWarning("usdiCreateMesh(): ctx must be set\n"); return nullptr; }
    if (!name) { usdiLogWarning("usdiCreateMesh(): name must be set\n"); return nullptr; }
    return new usdi::Mesh(ctx, parent, name);
}

usdiAPI void usdiMeshGetSummary(usdi::Mesh *mesh, usdi::MeshSummary *dst)
{
    usdiTraceFunc();
    if (!mesh || !dst) return;
    *dst = mesh->getSummary();
}

usdiAPI bool usdiMeshReadSample(usdi::Mesh *mesh, usdi::MeshData *dst, usdi::Time t, bool copy)
{
    usdiTraceFunc();
    if (!mesh || !dst) return false;
    return mesh->readSample(*dst, t, copy);
}
usdiAPI bool usdiMeshReadSampleAsync(usdi::Mesh *mesh, usdi::MeshData *dst, usdi::Time t, bool copy)
{
    usdiTraceFunc();
    if (!mesh || !dst) return false;
    usdi:: g_read_tasks.run([=]() { mesh->readSample(*dst, t, copy); });
    return true;
}

usdiAPI bool usdiMeshWriteSample(usdi::Mesh *mesh, const usdi::MeshData *src, usdi::Time t)
{
    usdiTraceFunc();
    if (!mesh || !src) return false;
    return mesh->writeSample(*src, t);
}
usdiAPI bool usdiMeshWriteSampleAsync(usdi::Mesh *mesh, const usdi::MeshData *src, usdi::Time t)
{
    usdiTraceFunc();
    if (!mesh || !src) return false;
    usdi::g_write_tasks.run([=]() { mesh->writeSample(*src, t); });
    return true;
}


usdiAPI usdi::Points* usdiAsPoints(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) return nullptr;
    return dynamic_cast<usdi::Points*>(schema);
}

usdiAPI usdi::Points* usdiCreatePoints(usdi::Context *ctx, usdi::Schema *parent, const char *name)
{
    usdiTraceFunc();
    if (!ctx) { usdiLogWarning("usdiCreatePoints(): ctx must be set\n"); return nullptr; }
    if (!name) { usdiLogWarning("usdiCreatePoints(): name must be set\n"); return nullptr; }
    return new usdi::Points(ctx, parent, name);
    return nullptr;
}

usdiAPI void usdiPointsGetSummary(usdi::Points *points, usdi::PointsSummary *dst)
{
    usdiTraceFunc();
    if (!points || !dst) return;
    *dst = points->getSummary();
}

usdiAPI bool usdiPointsReadSample(usdi::Points *points, usdi::PointsData *dst, usdi::Time t)
{
    usdiTraceFunc();
    if (!points || !dst) return false;
    return points->readSample(*dst, t);
}

usdiAPI bool usdiPointsWriteSample(usdi::Points *points, const usdi::PointsData *src, usdi::Time t)
{
    usdiTraceFunc();
    if (!points || !src) return false;
    return points->writeSample(*src, t);
}


usdiAPI const char* usdiAttrGetName(usdi::Attribute *attr)
{
    usdiTraceFunc();
    if (!attr) { return ""; }
    return attr->getName();
}

usdiAPI const char* usdiAttrGetTypeName(usdi::Attribute *attr)
{
    usdiTraceFunc();
    if (!attr) { return ""; }
    return attr->getTypeName();
}

usdiAPI usdi::AttributeType usdiAttrGetType(usdi::Attribute *attr)
{
    usdiTraceFunc();
    if (!attr) { return usdi::AttributeType::Unknown; }
    return attr->getType();
}

usdiAPI int usdiAttrGetArraySize(usdi::Attribute *attr, usdi::Time t)
{
    usdiTraceFunc();
    if (!attr) { return 0; }
    return (int)attr->getArraySize(t);
}

usdiAPI int usdiAttrGetNumSamples(usdi::Attribute *attr)
{
    usdiTraceFunc();
    if (!attr) { return 0; }
    return (int)attr->getNumSamples();
}

usdiAPI bool usdiAttrReadSample(usdi::Attribute *attr, void *dst, usdi::Time t)
{
    usdiTraceFunc();
    if (!attr) { return false; }
    return attr->getBuffered(dst, 1, t);
}

usdiAPI bool usdiAttrReadArraySample(usdi::Attribute *attr, void *dst, int size, usdi::Time t)
{
    usdiTraceFunc();
    if (!attr) { return false; }
    return attr->getBuffered(dst, size, t);
}

usdiAPI bool usdiAttrWriteSample(usdi::Attribute *attr, const void *src, usdi::Time t)
{
    usdiTraceFunc();
    if (!attr) { return false; }
    return attr->setBuffered(src, 1, t);
}

usdiAPI bool usdiAttrWriteArraySample(usdi::Attribute *attr, const void *src, int size, usdi::Time t)
{
    usdiTraceFunc();
    if (!attr) { return false; }
    return attr->setBuffered(src, size, t);
}

} // extern "C"
