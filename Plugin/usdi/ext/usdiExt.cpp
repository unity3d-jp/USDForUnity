#include "pch.h"

#include "usdiInternal.h"
#include "usdiAttribute.h"
#include "usdiSchema.h"
#include "usdiXform.h"
#include "usdiCamera.h"
#include "usdiMesh.h"
#include "usdiPoints.h"
#include "usdiContext.h"

#include "usdiExt.h"
#include "ext/usdiTask.h"



extern "C" {

usdiAPI void usdiTaskDestroy(usdi::Task *t)
{
    usdiTraceFunc();
    if (!t) { return; }
    delete t;
}

usdiAPI void usdiTaskRun(usdi::Task *t)
{
    usdiTraceFunc();
    if (!t) { return; }
    t->run();
}

usdiAPI bool usdiTaskIsRunning(usdi::Task *t)
{
    usdiTraceFunc();
    if (!t) { return false; }
    return t->isRunning();
}

usdiAPI void usdiTaskWait(usdi::Task *t)
{
    usdiTraceFunc();
    if (!t) { return; }
    t->wait();
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
