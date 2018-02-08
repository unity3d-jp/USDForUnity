#pragma once

#include "usdi.h"

namespace usdi {
    typedef size_t Handle;
    typedef void(usdiSTDCall *MonoDelegate)(void*);

    class Task;
    class IProgressReporter;
} // namespace usdi

extern "C" {

usdiAPI void            usdiTaskDestroy(usdi::Task *t);
usdiAPI void            usdiTaskRun(usdi::Task *t);
usdiAPI bool            usdiTaskIsRunning(usdi::Task *t);
usdiAPI void            usdiTaskWait(usdi::Task *t);
usdiAPI usdi::Task*     usdiTaskCreateMeshReadSample(usdi::Mesh *mesh, usdi::MeshData *dst, const usdi::Time *t);
usdiAPI usdi::Task*     usdiTaskCreatePointsReadSample(usdi::Points *points, usdi::PointsData *dst, const usdi::Time *t);
usdiAPI usdi::Task*     usdiTaskCreateAttrReadSample(usdi::Attribute *attr, usdi::AttributeData *dst, const usdi::Time *t);
usdiAPI usdi::Task*     usdiTaskCreateComposite(usdi::Task **tasks, int num);

// just for C#
usdiAPI int             usdiMemcmp(const void *a, const void *b, int size);
usdiAPI const char*     usdiIndexStringArray(const char **v, int i);
usdiAPI void            usdiMeshAssignRootBone(usdi::Mesh *mesh, usdi::MeshData *dst, const char *v);
usdiAPI void            usdiMeshAssignBones(usdi::Mesh *mesh, usdi::MeshData *dst, const char **v, int n);

} // extern "C"
