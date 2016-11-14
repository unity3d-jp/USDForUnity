#include "pch.h"
#include "usdiInternal.h"
#include "usdiAttribute.h"
#include "usdiSchema.h"
#include "usdiXform.h"
#include "usdiCamera.h"
#include "usdiMesh.h"
#include "usdiPoints.h"
#include "usdiContext.h"


#ifdef _WIN32
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
    #ifdef usdiDbgVTune
        #pragma comment(lib, "libittnotify.lib")
    #endif
#endif

namespace usdi {
    extern int g_debug_level;
} // namespace usdi

extern "C" {

usdiAPI void usdiSetDebugLevel(int l)
{
    usdi::g_debug_level = l;
}

usdiAPI usdi::Time usdiDefaultTime()
{
    return std::numeric_limits<double>::quiet_NaN();
}

usdiAPI void usdiSetPluginPath(const char *path)
{
    usdiTraceFunc();

    const char* env_name = "PXR_PLUGINPATH_NAME";
#ifdef _WIN32
    std::string tmp = env_name;
    tmp += "=";
    tmp += path;
    ::_putenv(tmp.c_str());
#else
    ::setenv(env_name, path, 1);
#endif
}



usdiAPI void usdiInitialize()
{
}

usdiAPI void usdiFinalize()
{
}

// Context interface

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

usdiAPI bool usdiSaveAs(usdi::Context *ctx, const char *path)
{
    usdiTraceFunc();
    if (!ctx || !path) return false;
    return ctx->saveAs(path);
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

usdiAPI void usdiUpdateAllSamples(usdi::Context *ctx, usdi::Time t)
{
    usdiTraceFunc();
    if (!ctx) return;

    usdiVTuneScope("usdiUpdateAllSamples");
    ctx->updateAllSamples(t);
}

usdiAPI void usdiInvalidateAllSamples(usdi::Context *ctx)
{
    usdiTraceFunc();
    if (!ctx) return;

    ctx->invalidateAllSamples();
}


// Schema interface

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
    return (int)schema->getNumAttributes();
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

usdiAPI bool usdiNeedsUpdate(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) { return false; }
    return schema->needsUpdate();
}


// Xform interface

usdiAPI usdi::Xform* usdiAsXform(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) return nullptr;
    return dynamic_cast<usdi::Xform*>(schema);
}

usdiAPI usdi::Xform* usdiCreateXform(usdi::Context *ctx, usdi::Schema *parent, const char *name)
{
    usdiTraceFunc();
    if (!ctx) { usdiLogWarning("usdiCreateXform(): ctx must not be null\n"); return nullptr; }
    if (!ctx->getUSDStage()) { usdiLogWarning("usdiCreateXform(): stage must not be null\n"); return nullptr; }
    if (!name) { usdiLogWarning("usdiCreateXform(): name must not be null\n"); return nullptr; }
    return new usdi::Xform(ctx, parent, name);
}

usdiAPI void usdiXformGetSummary(usdi::Xform *xf, usdi::XformSummary *dst)
{
    usdiTraceFunc();
    if (!xf || !dst) return;
    *dst = xf->getSummary();
}

usdiAPI bool usdiXformReadSample(usdi::Xform *xf, usdi::XformData *dst, usdi::Time t)
{
    usdiTraceFunc();
    if (!xf || !dst) return false;
    usdiVTuneScope("usdiXformReadSample");
    return xf->readSample(*dst, t);
}

usdiAPI bool usdiXformWriteSample(usdi::Xform *xf, const usdi::XformData *src, usdi::Time t)
{
    usdiTraceFunc();
    if (!xf || !src) return false;
    usdiVTuneScope("usdiXformWriteSample");
    return xf->writeSample(*src, t);
}


// Camera interface

usdiAPI usdi::Camera* usdiAsCamera(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) return nullptr;
    return dynamic_cast<usdi::Camera*>(schema);
}

usdiAPI usdi::Camera* usdiCreateCamera(usdi::Context *ctx, usdi::Schema *parent, const char *name)
{
    usdiTraceFunc();
    if (!ctx) { usdiLogWarning("usdiCreateCamera(): ctx must not be null\n"); return nullptr; }
    if (!ctx->getUSDStage()) { usdiLogWarning("usdiCreateCamera(): stage must not be null\n"); return nullptr; }
    if (!name) { usdiLogWarning("usdiCreateCamera(): name must not be null\n"); return nullptr; }
    return new usdi::Camera(ctx, parent, name);
}

usdiAPI void usdiCameraGetSummary(usdi::Camera *cam, usdi::CameraSummary *dst)
{
    usdiTraceFunc();
    if (!cam || !dst) return;
    *dst = cam->getSummary();
}

usdiAPI bool usdiCameraReadSample(usdi::Camera *cam, usdi::CameraData *dst, usdi::Time t)
{
    usdiTraceFunc();
    if (!cam || !dst) return false;
    usdiVTuneScope("usdiCameraReadSample");
    return cam->readSample(*dst, t);
}

usdiAPI bool usdiCameraWriteSample(usdi::Camera *cam, const usdi::CameraData *src, usdi::Time t)
{
    usdiTraceFunc();
    if (!cam || !src) return false;
    usdiVTuneScope("usdiCameraWriteSample");
    return cam->writeSample(*src, t);
}


// Mesh interface

usdiAPI usdi::Mesh* usdiAsMesh(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) return nullptr;
    return dynamic_cast<usdi::Mesh*>(schema);
}

usdiAPI usdi::Mesh* usdiCreateMesh(usdi::Context *ctx, usdi::Schema *parent, const char *name)
{
    usdiTraceFunc();
    if (!ctx) { usdiLogWarning("usdiCreateMesh(): ctx must not be null\n"); return nullptr; }
    if (!ctx->getUSDStage()) { usdiLogWarning("usdiCreateMesh(): stage must not be null\n"); return nullptr; }
    if (!name) { usdiLogWarning("usdiCreateMesh(): name must not be null\n"); return nullptr; }
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
    usdiVTuneScope("usdiMeshReadSample");
    return mesh->readSample(*dst, t, copy);
}

usdiAPI bool usdiMeshWriteSample(usdi::Mesh *mesh, const usdi::MeshData *src, usdi::Time t)
{
    usdiTraceFunc();
    if (!mesh || !src) return false;
    usdiVTuneScope("usdiMeshWriteSample");
    return mesh->writeSample(*src, t);
}


// Points interface

usdiAPI usdi::Points* usdiAsPoints(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) return nullptr;
    return dynamic_cast<usdi::Points*>(schema);
}

usdiAPI usdi::Points* usdiCreatePoints(usdi::Context *ctx, usdi::Schema *parent, const char *name)
{
    usdiTraceFunc();
    if (!ctx) { usdiLogWarning("usdiCreatePoints(): ctx must not be null\n"); return nullptr; }
    if (!ctx->getUSDStage()) { usdiLogWarning("usdiCreatePoints(): stage must not be null\n"); return nullptr; }
    if (!name) { usdiLogWarning("usdiCreatePoints(): name must not be null\n"); return nullptr; }
    return new usdi::Points(ctx, parent, name);
}

usdiAPI void usdiPointsGetSummary(usdi::Points *points, usdi::PointsSummary *dst)
{
    usdiTraceFunc();
    if (!points || !dst) return;
    *dst = points->getSummary();
}

usdiAPI bool usdiPointsReadSample(usdi::Points *points, usdi::PointsData *dst, usdi::Time t, bool copy)
{
    usdiTraceFunc();
    if (!points || !dst) return false;
    usdiVTuneScope("usdiPointsReadSample");
    return points->readSample(*dst, t, copy);
}

usdiAPI bool usdiPointsWriteSample(usdi::Points *points, const usdi::PointsData *src, usdi::Time t)
{
    usdiTraceFunc();
    if (!points || !src) return false;
    usdiVTuneScope("usdiPointsWriteSample");
    return points->writeSample(*src, t);
}


// Attribute interface

usdiAPI usdi::Schema* usdiAttrGetParent(usdi::Attribute *attr)
{
    usdiTraceFunc();
    if (!attr) { return nullptr; }
    return attr->getParent();
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

usdiAPI void usdiAttrGetSummary(usdi::Attribute *attr, usdi::AttributeSummary *dst)
{
    usdiTraceFunc();
    if (!attr) { return; }
    *dst = attr->getSummary();
}

usdiAPI bool usdiAttrReadSample(usdi::Attribute *attr, usdi::AttributeData *dst, usdi::Time t, bool copy)
{
    usdiTraceFunc();
    if (!attr || !dst) { return false; }
    return attr->readSample(*dst, t, copy);
}

usdiAPI bool usdiAttrWriteSample(usdi::Attribute *attr, const usdi::AttributeData *src, usdi::Time t)
{
    usdiTraceFunc();
    if (!attr || !src) { return false; }
    return attr->writeSample(*src, t);
}


} // extern "C"
