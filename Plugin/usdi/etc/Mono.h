#pragma once


typedef unsigned __int32    guint32;
typedef int                 gboolean;
typedef void*               gpointer;

typedef void MonoDomain;
typedef void MonoAssembly;
typedef void MonoImage;
typedef void MonoThread;

typedef void MonoClass;
typedef void MonoMethod;
typedef void MonoObject;
typedef void MonoArray;

extern void *g_mono_dll;

extern MonoDomain*  (*mono_domain_get)(void);
extern MonoAssembly*(*mono_domain_assembly_open)(MonoDomain *domain, const char *assemblyName);
extern MonoImage*   (*mono_assembly_get_image)(MonoAssembly *assembly);

extern MonoThread*  (*mono_thread_current)(void);
extern MonoThread*  (*mono_thread_attach)(MonoDomain *domain);
extern void         (*mono_thread_detach)(MonoThread *thread);
extern void         (*mono_thread_suspend_all_other_threads)();
extern void         (*mono_thread_abort_all_other_threads)();
extern void         (*mono_jit_thread_attach)(MonoDomain *domain);

extern void         (*mono_add_internal_call)(const char *name, void *method);
extern MonoObject*  (*mono_runtime_invoke)(MonoMethod *method, MonoObject *obj, void **params, void **exc);

extern MonoClass*   (*mono_class_from_name)(MonoImage *image, const char *namespaceString, const char *classnameString);
extern MonoMethod*  (*mono_class_get_method_from_name)(MonoClass *klass, const char *name, int param_count);

extern MonoClass*   (*mono_object_get_class)(MonoObject *obj);
extern gpointer     (*mono_object_unbox)(MonoObject *obj);

extern guint32      (*mono_gchandle_new)(MonoObject *obj, gboolean pinned);
extern void         (*mono_gchandle_free)(guint32 gchandle);
