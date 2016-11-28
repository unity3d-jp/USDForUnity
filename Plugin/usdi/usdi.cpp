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
    return ctx->getRoot();
}
usdiAPI int usdiGetNumMasters(usdi::Context *ctx)
{
    usdiTraceFunc();
    if (!ctx) return 0;
    return ctx->getNumMasters();
}
usdiAPI usdi::Schema* usdiGetMaster(usdi::Context *ctx, int i)
{
    usdiTraceFunc();
    if (!ctx) return nullptr;
    return ctx->getMaster(i);
}
usdiAPI usdi::Schema* usdiFindSchema(usdi::Context *ctx, const char *prim_path)
{
    usdiTraceFunc();
    if (!ctx) return nullptr;
    return ctx->findSchema(prim_path);
}

usdiAPI usdi::Schema* usdiCreateOverride(usdi::Context *ctx, const char *prim_path)
{
    usdiTraceFunc();
    if (!ctx) { usdiLogError("usdiCreateOverride(): ctx is null\n"); return nullptr; }
    return ctx->createOverride(prim_path);
}
usdiAPI usdi::Xform* usdiCreateXform(usdi::Context *ctx, usdi::Schema *parent, const char *name)
{
    usdiTraceFunc();
    if (!ctx) { usdiLogError("usdiCreateXform(): ctx is null\n"); return nullptr; }
    return ctx->createSchema<usdi::Xform>(parent, name);
}
usdiAPI usdi::Camera* usdiCreateCamera(usdi::Context *ctx, usdi::Schema *parent, const char *name)
{
    usdiTraceFunc();
    if (!ctx) { usdiLogError("usdiCreateCamera(): ctx is null\n"); return nullptr; }
    return ctx->createSchema<usdi::Camera>(parent, name);
}
usdiAPI usdi::Mesh* usdiCreateMesh(usdi::Context *ctx, usdi::Schema *parent, const char *name)
{
    usdiTraceFunc();
    if (!ctx) { usdiLogError("usdiCreateMesh(): ctx is null\n"); return nullptr; }
    return ctx->createSchema<usdi::Mesh>(parent, name);
}
usdiAPI usdi::Points* usdiCreatePoints(usdi::Context *ctx, usdi::Schema *parent, const char *name)
{
    usdiTraceFunc();
    if (!ctx) { usdiLogError("usdiCreatePoints(): ctx is null\n"); return nullptr; }
    return ctx->createSchema<usdi::Points>(parent, name);
}

usdiAPI void usdiFlatten(usdi::Context *ctx)
{
    usdiTraceFunc();
    if (!ctx) return;
    ctx->flatten();
}

usdiAPI void usdiNotifyForceUpdate(usdi::Context *ctx)
{
    usdiTraceFunc();
    if (!ctx) return;
    ctx->notifyForceUpdate();
}

usdiAPI void usdiUpdateAllSamples(usdi::Context *ctx, usdi::Time t)
{
    usdiTraceFunc();
    if (!ctx) return;

    usdiVTuneScope("usdiUpdateAllSamples");
    ctx->updateAllSamples(t);
}
usdiAPI void usdiRebuildSchemaTree(usdi::Context *ctx)
{
    usdiTraceFunc();
    if (!ctx) return;
    ctx->rebuildSchemaTree();
}


// Schema interface

usdiAPI int usdiPrimGetID(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) { return 0; }
    return schema->getID();
}
usdiAPI const char* usdiPrimGetPath(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) { return ""; }
    return schema->getPath();
}
usdiAPI const char* usdiPrimGetName(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) { return ""; }
    return schema->getName();
}
usdiAPI const char* usdiPrimGetUsdTypeName(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) { return ""; }
    return schema->getUsdTypeName();
}

usdiAPI usdi::Schema* usdiPrimGetMaster(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) { return nullptr; }
    return schema->getMaster();
}
usdiAPI int usdiPrimGetNumInstances(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) { return 0; }
    return schema->getNumInstances();
}
usdiAPI usdi::Schema* usdiPrimGetInstance(usdi::Schema *schema, int i)
{
    usdiTraceFunc();
    if (!schema) { return nullptr; }
    return schema->getInstance(i);
}

usdiAPI bool usdiPrimIsInstance(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) { return nullptr; }
    return schema->isInstance();
}
usdiAPI bool usdiPrimIsInstanceable(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) { return nullptr; }
    return schema->isInstanceable();
}
usdiAPI bool usdiPrimIsMaster(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) { return nullptr; }
    return schema->isMaster();
}
usdiAPI void usdiPrimSetInstanceable(usdi::Schema *schema, bool v)
{
    usdiTraceFunc();
    if (!schema) { return; }
    schema->setInstanceable(v);
}
usdiAPI bool usdiPrimAddReference(usdi::Schema *schema, const char *asset_path, const char *prim_path)
{
    usdiTraceFunc();
    if (!schema) { return false; }
    return schema->addReference(asset_path, prim_path);
}

usdiAPI bool usdiPrimHasPayload(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) { return false; }
    return schema->hasPayload();
}
usdiAPI void usdiPrimLoadPayload(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) { return; }
    schema->loadPayload();
}
usdiAPI void usdiPrimUnloadPayload(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) { return; }
    schema->unloadPayload();
}
usdiAPI bool usdiPrimSetPayload(usdi::Schema *schema, const char *asset_path, const char *prim_path)
{
    usdiTraceFunc();
    if (!schema) { return false; }
    return schema->setPayload(asset_path, prim_path);
}

usdiAPI usdi::Schema* usdiPrimGetParent(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) { return nullptr; }
    return schema->getParent();
}
usdiAPI int usdiPrimGetNumChildren(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) return 0;
    return (int)schema->getNumChildren();
}
usdiAPI usdi::Schema* usdiPrimGetChild(usdi::Schema *schema, int i)
{
    usdiTraceFunc();
    if (!schema) return nullptr;
    return schema->getChild(i);

}
usdiAPI int usdiPrimGetNumAttributes(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) { return 0; }
    return (int)schema->getNumAttributes();
}
usdiAPI usdi::Attribute* usdiPrimGetAttribute(usdi::Schema *schema, int i)
{
    usdiTraceFunc();
    if (!schema) { return nullptr; }
    return schema->getAttribute(i);
}
usdiAPI usdi::Attribute* usdiPrimFindAttribute(usdi::Schema *schema, const char *name)
{
    usdiTraceFunc();
    if (!schema) { return nullptr; }
    return schema->findAttribute(name);
}
usdiAPI usdi::Attribute* usdiPrimCreateAttribute(usdi::Schema *schema, const char *name, usdi::AttributeType type)
{
    usdiTraceFunc();
    if (!schema) { return nullptr; }
    return schema->createAttribute(name, type);
}

usdiAPI int usdiPrimGetNumVariantSets(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) { return 0; }
    return schema->getNumVariantSets();
}
usdiAPI const char* usdiPrimGetVariantSetName(usdi::Schema *schema, int iset)
{
    usdiTraceFunc();
    if (!schema) { return ""; }
    return schema->getVariantSetName(iset);
}
usdiAPI int usdiPrimGetNumVariants(usdi::Schema *schema, int iset)
{
    usdiTraceFunc();
    if (!schema) { return 0; }
    return schema->getNumVariants(iset);
}
usdiAPI const char* usdiPrimGetVariantName(usdi::Schema *schema, int iset, int ival)
{
    usdiTraceFunc();
    if (!schema) { return ""; }
    return schema->getVariantName(iset, ival);
}
usdiAPI int usdiPrimGetVariantSelection(usdi::Schema *schema, int iset)
{
    usdiTraceFunc();
    if (!schema) { return false; }
    return schema->getVariantSelection(iset);
}
usdiAPI bool usdiPrimSetVariantSelection(usdi::Schema *schema, int iset, int ival)
{
    usdiTraceFunc();
    if (!schema) { return false; }
    return schema->setVariantSelection(iset, ival);
}
usdiAPI int usdiPrimFindVariantSet(usdi::Schema *schema, const char *name)
{
    usdiTraceFunc();
    if (!schema) { return -1; }
    return schema->findVariantSet(name);
}
usdiAPI int usdiPrimFindVariant(usdi::Schema *schema, int iset, const char *name)
{
    usdiTraceFunc();
    if (!schema) { return -1; }
    return schema->findVariant(iset, name);
}
usdiAPI int usdiPrimCreateVariantSet(usdi::Schema *schema, const char *name)
{
    usdiTraceFunc();
    if (!schema) { return -1; }
    return schema->createVariantSet(name);
}
usdiAPI int usdiPrimCreateVariant(usdi::Schema *schema, int iset, const char *name)
{
    usdiTraceFunc();
    if (!schema) { return -1; }
    return schema->createVariant(iset, name);
}
usdiAPI bool usdiPrimBeginEditVariant(usdi::Schema *schema, int iset, int ival)
{
    usdiTraceFunc();
    if (!schema) { return false; }
    return schema->beginEditVariant(iset, ival);
}
usdiAPI void usdiPrimEndEditVariant(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) { return; }
    return schema->endEditVariant();
}

usdiAPI usdi::UpdateFlags usdiPrimGetUpdateFlags(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) { return{ 0 }; }
    return schema->getUpdateFlags();
}
usdiAPI usdi::UpdateFlags usdiPrimGetUpdateFlagsPrev(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) { return{ 0 }; }
    return schema->getUpdateFlagsPrev();
}

usdiAPI void usdiPrimUpdateSample(usdi::Schema *schema, usdi::Time t)
{
    usdiTraceFunc();
    if (!schema) { return; }
    return schema->updateSample(t);
}
usdiAPI void* usdiPrimGetUserData(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) { return nullptr; }
    return schema->getUserData();
}
usdiAPI void usdiPrimSetUserData(usdi::Schema *schema, void *data)
{
    usdiTraceFunc();
    if (!schema) { return; }
    return schema->setUserData(data);
}


// Xform interface

usdiAPI usdi::Xform* usdiAsXform(usdi::Schema *schema)
{
    usdiTraceFunc();
    if (!schema) return nullptr;
    return schema->as<usdi::Xform*>();
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
    return schema->as<usdi::Camera*>();
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
    return schema->as<usdi::Mesh*>();
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
    return schema->as<usdi::Points*>();
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
