#include "pch.h"

#ifdef usdiEnableUnityExtension
#include "usdiInternal.h"
#include "usdiAttribute.h"
#include "usdiSchema.h"
#include "usdiXform.h"
#include "usdiCamera.h"
#include "usdiMesh.h"
#include "usdiPoints.h"
#include "usdiContext.h"

#include "etc/Mono.h"
#include "usdiExt.h"
#include "ext/usdiTask.h"
#include "ext/usdiComponentUpdater.h"



extern "C" {

usdiAPI bool usdiVtxCmdIsAvailable()
{
#ifdef usdiEnableGraphicsInterface
    return gi::GetGraphicsInterface() != nullptr;
#else
    return false;
#endif
}

usdiAPI usdi::Handle usdiVtxCmdCreate(const char *dbg_name)
{
#ifdef usdiEnableGraphicsInterface
    usdiTraceFunc();
    return usdi::VertexCommandManager::getInstance().createCommand(dbg_name);
#else
    return 0;
#endif
}

usdiAPI void usdiVtxCmdDestroy(usdi::Handle h)
{
#ifdef usdiEnableGraphicsInterface
    usdiTraceFunc();
    usdi::VertexCommandManager::getInstance().destroyCommand(h);
#endif
}

usdiAPI void usdiVtxCmdUpdate(usdi::Handle h, const usdi::MeshData *src, void *vb, void *ib)
{
#ifdef usdiEnableGraphicsInterface
    usdiTraceFunc();
    usdi::VertexCommandManager::getInstance().update(h, src, vb, ib);
#endif
}

usdiAPI void usdiVtxCmdUpdateSub(usdi::Handle h, const usdi::SubmeshData *src, void *vb, void *ib)
{
#ifdef usdiEnableGraphicsInterface
    usdiTraceFunc();
    usdi::VertexCommandManager::getInstance().update(h, src, vb, ib);
#endif
}

usdiAPI void usdiVtxCmdProcess()
{
#ifdef usdiEnableGraphicsInterface
    usdiTraceFunc();
    usdiVTuneScope("usdiVtxCmdProcess");
    usdi::VertexCommandManager::getInstance().process();
#endif
}

usdiAPI void usdiVtxCmdWait()
{
#ifdef usdiEnableGraphicsInterface
    usdiTraceFunc();
    usdiVTuneScope("usdiVtxCmdWait");
    usdi::VertexCommandManager::getInstance().wait();
#endif
}




usdiAPI void usdiTaskDestroy(usdi::Task *t)
{
    usdiTraceFunc();
    delete t;
}

usdiAPI void usdiTaskRun(usdi::Task *t)
{
    usdiTraceFunc();
    t->run();
}

usdiAPI bool usdiTaskIsRunning(usdi::Task *t)
{
    usdiTraceFunc();
    return t->isRunning();
}

usdiAPI void usdiTaskWait(usdi::Task *t)
{
    usdiTraceFunc();
    t->wait();
}

usdiAPI usdi::Task* usdiTaskCreateMonoDelegate(usdi::MonoDelegate func, void *arg, const char *name)
{
    usdiTraceFunc();
    return new usdi::Task([=]() {
        usdi::MonoThreadScope mts;
        func(arg);
    }, name);
}

usdiAPI usdi::Task* usdiTaskCreateMeshReadSample(usdi::Mesh *mesh, usdi::MeshData *dst, const usdi::Time *t)
{
    usdiTraceFunc();
    if (!mesh || !dst || !t) { return 0; }
    return new usdi::Task([=]() {
        mesh->readSample(*dst, *t, true);
    });
}

usdiAPI usdi::Task* usdiTaskCreatePointsReadSample(usdi::Points *points, usdi::PointsData *dst, const usdi::Time *t)
{
    usdiTraceFunc();
    if (!points || !dst || !t) { return 0; }
    return new usdi::Task([=]() {
        points->readSample(*dst, *t, true);
    });
}

usdiAPI usdi::Task* usdiTaskCreateAttrReadSample(usdi::Attribute *attr, usdi::AttributeData *dst, const usdi::Time *t)
{
    usdiTraceFunc();
    if (!attr || !dst || !t) { return 0; }
    return new usdi::Task([=]() {
        attr->readSample(*dst, *t, true);
    });
}

usdiAPI usdi::Task* usdiTaskCreateComposite(usdi::Task **tasks, int num)
{
    usdiTraceFunc();
    if (!tasks || num == 0) { return 0; }
    return new usdi::Task([=]() {
        for (int i = 0; i < num; ++i) {
            tasks[i]->run(false);
        }
    });
}

usdiAPI int usdiMemcmp(const void *a, const void *b, int size)
{
    return memcmp(a, b, size);
}
usdiAPI const char* usdiIndexStringArray(const char **v, int i)
{
    if (!v) { return ""; }
    return v[i];
}
usdiAPI void usdiMeshAssignRootBone(usdi::Mesh *mesh, usdi::MeshData *dst, const char *v)
{
    if (!mesh || !dst) { return; }
    mesh->assignRootBone(*dst, v);
}
usdiAPI void usdiMeshAssignBones(usdi::Mesh *mesh, usdi::MeshData *dst, const char **v, int n)
{
    if (!mesh || !dst) { return; }
    mesh->assignBones(*dst, v, n);
}

} // extern "C"
#endif // usdiEnableUnityExtension
