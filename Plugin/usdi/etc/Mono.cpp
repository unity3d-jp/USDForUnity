#include "pch.h"
#include "Mono.h"
#ifdef _WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif


// mono functions
void *g_mono_dll;
MonoDomain *g_mono_domain;
MonoDomain* (*mono_domain_get)(void);
MonoThread* (*mono_thread_current)(void);
MonoThread* (*mono_thread_attach)(MonoDomain *domain);
void (*mono_thread_detach)(MonoThread *thread);
void (*mono_thread_suspend_all_other_threads)();
void (*mono_thread_abort_all_other_threads)();
void (*mono_jit_thread_attach)(MonoDomain *domain);
void (*mono_add_internal_call)(const char *name, void *method);


static void initialize_mono_functions()
{
#ifdef _WIN32
    auto mono = ::GetModuleHandleA("mono.dll");
    g_mono_dll = mono;
    if (mono) {
        (void*&)mono_domain_get = ::GetProcAddress(mono, "mono_domain_get");
        (void*&)mono_thread_current = ::GetProcAddress(mono, "mono_thread_current");
        (void*&)mono_thread_attach = ::GetProcAddress(mono, "mono_thread_attach");
        (void*&)mono_thread_detach = ::GetProcAddress(mono, "mono_thread_detach");
        (void*&)mono_thread_suspend_all_other_threads = ::GetProcAddress(mono, "mono_thread_suspend_all_other_threads");
        (void*&)mono_thread_abort_all_other_threads = ::GetProcAddress(mono, "mono_thread_abort_all_other_threads");
        (void*&)mono_jit_thread_attach = ::GetProcAddress(mono, "mono_jit_thread_attach");
        (void*&)mono_add_internal_call = ::GetProcAddress(mono, "mono_add_internal_call");
    }
#else
    // todo
#endif
}

static struct _initialize_mono_functions {
    _initialize_mono_functions() { initialize_mono_functions(); }
} g_initialize_mono_functions;

