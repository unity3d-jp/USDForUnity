#include "pch.h"
#ifdef usdiEnableUnityExtension
#include "usdiInternal.h"
#include "ext/usdiExt.h"
#include "GraphicsInterface/GraphicsInterface.h"
#include "GraphicsInterface/giUnityPluginImpl.h"
#include "PluginAPI/IUnityGraphics.h"

static IUnityInterfaces* g_unity_interface;

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
UnityPluginLoad(IUnityInterfaces* unityInterfaces)
{
    g_unity_interface = unityInterfaces;
    gi::UnityPluginLoad(unityInterfaces);
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
UnityPluginUnload()
{
    gi::UnityPluginUnload();
}

static void UNITY_INTERFACE_API UnityRenderEventFunc(int /*eventID*/)
{
    usdiVtxCmdProcess();
}

extern "C" UNITY_INTERFACE_EXPORT 
UnityRenderingEvent usdiGetRenderEventFunc()
{
    return UnityRenderEventFunc;
}


extern "C" UNITY_INTERFACE_EXPORT IUnityInterfaces* GetUnityInterface()
{
    return g_unity_interface;
}
#endif // usdiEnableUnityExtension
