#include "pch.h"

#include "usdiInternal.h"
#include "etc/Hook.h"
#include "etc/Mono.h"
#include "etc/MonoWrapper.h"

#ifdef _WIN32
BOOL WINAPI DllMain(HINSTANCE /*hinstDLL*/, DWORD fdwReason, LPVOID /*lpvReserved*/)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
//#ifndef usdiEnableMonoBindingThreadGuard
        // redirect _mono_thread_suspend_all_other_threads to _mono_thread_abort_all_other_threads to avoid
        // Mono wait TBB worker threads forever...
#ifdef usdiEnableMonoBinding
        if (g_mono_dll) {
            ForceWrite(_mono_thread_suspend_all_other_threads, 14, [=]() {
                EmitJumpInstruction(_mono_thread_suspend_all_other_threads, _mono_thread_abort_all_other_threads);
            });
        }
#endif // usdiEnableMonoBinding
//#endif
    }
    return TRUE;
}
#elif defined(__APPLE__)

void ImportMonoFunctions();

__attribute__((constructor))
void DllMain()
{
#ifdef usdiEnableMonoBinding
    ImportMonoFunctions();
    if (g_mono_dll) {
        ForceWrite((void*)_mono_thread_suspend_all_other_threads, 14, [=]() {
            EmitJumpInstruction((void*)_mono_thread_suspend_all_other_threads, (void*)_mono_thread_abort_all_other_threads);
        });
    }
#endif // usdiEnableMonoBinding
}

#endif // _WIN32
