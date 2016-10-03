#pragma once


typedef void MonoThread;
typedef void MonoDomain;

extern void *g_mono_dll;
extern MonoDomain* (*mono_domain_get)(void);
extern MonoThread* (*mono_thread_current)(void);
extern MonoThread* (*mono_thread_attach)(MonoDomain *domain);
extern void(*mono_thread_detach)(MonoThread *thread);
extern void(*mono_thread_suspend_all_other_threads)();
extern void(*mono_thread_abort_all_other_threads)();
extern void(*mono_jit_thread_attach)(MonoDomain *domain);
extern void(*mono_add_internal_call)(const char *name, void *method);


