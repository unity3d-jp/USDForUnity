#include "pch.h"
#include "Mono.h"
#ifdef _WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif


// dummy
MonoDomain* mono_domain_get_dummy(void) { return nullptr; }
MonoThread* mono_thread_attach_dummy(MonoDomain *domain) { return nullptr; }
void mono_thread_detach_dummy(MonoThread *thread) {}
void mono_thread_suspend_all_other_threads_dummy() {}
void mono_jit_thread_attach_dummy(MonoDomain *domain) {}

// mono functions
void *g_mono_dll;
MonoDomain *g_mono_domain;
MonoDomain* (*mono_domain_get)(void) = mono_domain_get_dummy;
MonoThread* (*mono_thread_attach)(MonoDomain *domain) = mono_thread_attach_dummy;
void (*mono_thread_detach)(MonoThread *thread) = mono_thread_detach_dummy;
void (*mono_thread_suspend_all_other_threads)() = mono_thread_suspend_all_other_threads_dummy;
void (*mono_thread_abort_all_other_threads)() = mono_thread_suspend_all_other_threads_dummy;
void (*mono_jit_thread_attach)(MonoDomain *domain) = mono_jit_thread_attach_dummy;


static void initialize_mono_functions()
{
#ifdef _WIN32
    auto mono = ::GetModuleHandleA("mono.dll");
    g_mono_dll = mono;
    if (mono) {
        (void*&)mono_domain_get = ::GetProcAddress(mono, "mono_domain_get");
        (void*&)mono_thread_attach = ::GetProcAddress(mono, "mono_thread_attach");
        (void*&)mono_thread_detach = ::GetProcAddress(mono, "mono_thread_detach");
        (void*&)mono_thread_suspend_all_other_threads = ::GetProcAddress(mono, "mono_thread_suspend_all_other_threads");
        (void*&)mono_thread_abort_all_other_threads = ::GetProcAddress(mono, "mono_thread_abort_all_other_threads");
        (void*&)mono_jit_thread_attach = ::GetProcAddress(mono, "mono_jit_thread_attach");
    }
#else
    // todo
#endif
    g_mono_domain = mono_domain_get();
}

static struct _initialize_mono_functions {
    _initialize_mono_functions() { initialize_mono_functions(); }
} g_initialize_mono_functions;


MonoScope::MonoScope()
{
    m_mono_thread = mono_thread_attach(g_mono_domain);
}

MonoScope::~MonoScope()
{
    mono_thread_detach(m_mono_thread);
}
