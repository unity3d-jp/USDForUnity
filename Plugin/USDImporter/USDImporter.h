#pragma once

#define usdiCLinkage extern "C"
#ifdef _WIN32
    #ifndef usdiStaticLink
        #ifdef usdiImpl
            #define usdiExport __declspec(dllexport)
        #else
            #define usdiExport __declspec(dllimport)
        #endif
    #else
        #define usdiExport
    #endif
#else
    #define usdiExport
#endif

namespace usdi {

class Context;

class Schema;
class Xform;
class Camera;
class PolyMesh;

class Sample;
class XformSample;
class CameraSample;
class PolyMeshSample;

} // namespace usdi

usdi::Context* usdiOpen(const char *path);
