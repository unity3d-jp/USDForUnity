#include "pch.h"
#include "Foundation.h"
#include "GraphicsDevice.h"


namespace utj {
GraphicsDevice* CreateGraphicsDeviceOpenGL();
GraphicsDevice* CreateGraphicsDeviceD3D9(void *device);
GraphicsDevice* CreateGraphicsDeviceD3D11(void *device);
static GraphicsDevice *g_gfx_device;
} // namespace utj
using namespace utj;

utjCLinkage utjExport GraphicsDevice* GetGraphicsDevice()
{
    return g_gfx_device;
}


#ifdef utjSupportOpenGL
utjCLinkage utjExport void GfxInitializeOpenGL()
{
    if (g_gfx_device != nullptr) {
        utjDebugLog("already initialized");
        return;
    }
    g_gfx_device = CreateGraphicsDeviceOpenGL();
}
#endif

#ifdef utjSupportD3D9
utjCLinkage utjExport void GfxInitializeD3D9(void *device)
{
    if (g_gfx_device != nullptr) {
        utjDebugLog("already initialized");
        return;
    }
    g_gfx_device = CreateGraphicsDeviceD3D9(device);
}
#endif

#ifdef utjSupportD3D11
utjCLinkage utjExport void GfxInitializeD3D11(void *device)
{
    if (g_gfx_device != nullptr) {
        utjDebugLog("already initialized");
        return;
    }
    g_gfx_device = CreateGraphicsDeviceD3D11(device);
}
#endif

utjCLinkage utjExport void GfxFinalize()
{
    delete g_gfx_device;
    g_gfx_device = nullptr;
}

utjCLinkage utjExport void GfxSync()
{
    if (g_gfx_device) {
        g_gfx_device->sync();
    }
}



#ifndef utjStaticLink

#include "PluginAPI/IUnityGraphics.h"
#ifdef utjSupportD3D9
    #include <d3d9.h>
    #include "PluginAPI/IUnityGraphicsD3D9.h"
#endif
#ifdef utjSupportD3D11
    #include <d3d11.h>
    #include "PluginAPI/IUnityGraphicsD3D11.h"
#endif
#ifdef utjSupportD3D12
    #include <d3d12.h>
    #include "PluginAPI/IUnityGraphicsD3D12.h"
#endif

namespace utj {

static IUnityInterfaces* g_unity_interface;

static void UNITY_INTERFACE_API UnityOnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType)
{
    if (eventType == kUnityGfxDeviceEventInitialize) {
        auto unity_gfx = g_unity_interface->Get<IUnityGraphics>();
        auto api = unity_gfx->GetRenderer();

#ifdef utjSupportD3D11
        if (api == kUnityGfxRendererD3D11) {
            GfxInitializeD3D11(g_unity_interface->Get<IUnityGraphicsD3D11>()->GetDevice());
        }
#endif
#ifdef utjSupportD3D9
        if (api == kUnityGfxRendererD3D9) {
            GfxInitializeD3D9(g_unity_interface->Get<IUnityGraphicsD3D9>()->GetDevice());
        }
#endif
#ifdef utjSupportOpenGL
        if (api == kUnityGfxRendererOpenGL ||
            api == kUnityGfxRendererOpenGLCore ||
            api == kUnityGfxRendererOpenGLES20 ||
            api == kUnityGfxRendererOpenGLES30)
        {
            GfxInitializeOpenGL();
        }
#endif
    }
    else if (eventType == kUnityGfxDeviceEventShutdown) {
        GfxFinalize();
    }
}
} // namespace utj


using namespace utj;

// user must implement this
void RenderEventCallback(int id);

static void UNITY_INTERFACE_API UnityRenderEvent(int eventID)
{
    RenderEventCallback(eventID);
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
UnityPluginLoad(IUnityInterfaces* unityInterfaces)
{
    g_unity_interface = unityInterfaces;
    g_unity_interface->Get<IUnityGraphics>()->RegisterDeviceEventCallback(UnityOnGraphicsDeviceEvent);
    UnityOnGraphicsDeviceEvent(kUnityGfxDeviceEventInitialize);
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
UnityPluginUnload()
{
    auto unity_gfx = g_unity_interface->Get<IUnityGraphics>();
    unity_gfx->UnregisterDeviceEventCallback(UnityOnGraphicsDeviceEvent);
}

utjCLinkage utjExport UnityRenderingEvent GetRenderEventFunc()
{
    return UnityRenderEvent;
}

utjCLinkage utjExport IUnityInterfaces* GetUnityInterface()
{
    return g_unity_interface;
}

#ifdef utjWindows
#include <windows.h>
typedef IUnityInterfaces* (*GetUnityInterfaceT)();
extern const char *utjModuleName;

void GfxForceInitialize()
{
    // PatchLibrary で突っ込まれたモジュールは UnityPluginLoad() が呼ばれないので、
    // 先にロードされているモジュールからインターフェースをもらって同等の処理を行う。
    HMODULE m = ::GetModuleHandleA(utjModuleName);
    if (m) {
        auto proc = (GetUnityInterfaceT)::GetProcAddress(m, "GetUnityInterface");
        if (proc) {
            auto *iface = proc();
            if (iface) {
                UnityPluginLoad(iface);
            }
        }
    }
}
#endif

#endif // utjStaticLink
