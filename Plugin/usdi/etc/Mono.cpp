#include "pch.h"
#include "Mono.h"
#ifdef _WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif


// mono functions
void *g_mono_dll;

void            (*g_free)(void *ptr);

MonoDomain*     (*mono_domain_get)(void);
MonoAssembly*   (*mono_domain_assembly_open)(MonoDomain *domain, const char *assemblyName);
MonoImage*      (*mono_assembly_get_image)(MonoAssembly *assembly);
gboolean        (*mono_assembly_fill_assembly_name)(MonoImage *image, MonoAssemblyName *aname);
MonoAssembly*   (*mono_image_get_assembly)(MonoImage *image);
char*           (*mono_stringify_assembly_name)(MonoAssemblyName *aname);

MonoThread*     (*mono_thread_current)(void);
MonoThread*     (*mono_thread_attach)(MonoDomain *domain);
void            (*mono_thread_detach)(MonoThread *thread);
void            (*mono_thread_suspend_all_other_threads)();
void            (*mono_thread_abort_all_other_threads)();
void            (*mono_jit_thread_attach)(MonoDomain *domain);

void            (*mono_add_internal_call)(const char *name, void *method);
MonoObject*     (*mono_runtime_invoke)(MonoMethod *method, MonoObject *obj, void **params, void **exc);

char*           (*mono_type_get_name)(MonoType *type);
MonoClass*      (*mono_type_get_class)(MonoType *type);

MonoClass*      (*mono_class_from_name)(MonoImage *image, const char *namespaceString, const char *classnameString);
const char*     (*mono_class_get_name)(MonoClass *klass);
const char*     (*mono_class_get_namespace)(MonoClass *klass);
MonoType*       (*mono_class_get_type)(MonoClass *klass);
MonoMethod*     (*mono_class_get_method_from_name)(MonoClass *klass, const char *name, int param_count);
MonoClassField* (*mono_class_get_field_from_name)(MonoClass *klass, const char *name);
MonoMethod*     (*mono_class_inflate_generic_method)(MonoMethod *method, MonoGenericContext *context);
gboolean        (*mono_class_is_subclass_of)(MonoClass *klass, MonoClass *klassc, gboolean check_interfaces);
MonoProperty*   (*mono_class_get_property_from_name)(MonoClass *klass, const char *name);
MonoMethod*     (*mono_class_get_methods)(MonoClass* klass, gpointer *iter);
MonoClassField* (*mono_class_get_fields)(MonoClass* klass, gpointer *iter);
MonoProperty*   (*mono_class_get_properties)(MonoClass* klass, gpointer *iter);
MonoClass*      (*mono_class_get_parent)(MonoClass *klass);
MonoImage*      (*mono_class_get_image)(MonoClass *klass);

const char*     (*mono_method_get_name)(MonoMethod *method);
MonoMethodSignature* (*mono_method_signature)(MonoMethod *method);
guint32         (*mono_signature_get_param_count)(MonoMethodSignature *sig);
MonoType*       (*mono_signature_get_params)(MonoMethodSignature *sig, gpointer *iter);

const char*     (*mono_field_get_name)(MonoClassField *field);
MonoType*       (*mono_field_get_type)(MonoClassField *field);
MonoClass*      (*mono_field_get_parent)(MonoClassField *field);
guint32         (*mono_field_get_flags)(MonoClassField *field);
guint32         (*mono_field_get_offset)(MonoClassField *field);
const char*     (*mono_field_get_data)(MonoClassField *field);
void            (*mono_field_get_value)(MonoObject *obj, MonoClassField *field, void *value);
void            (*mono_field_set_value)(MonoObject *obj, MonoClassField *field, void *value);

const char*     (*mono_property_get_name)(MonoProperty *prop);
MonoMethod*     (*mono_property_get_set_method)(MonoProperty *prop);
MonoMethod*     (*mono_property_get_get_method)(MonoProperty *prop);
MonoClass*      (*mono_property_get_parent)(MonoProperty *prop);

MonoObject*     (*mono_object_new)(MonoDomain *domain, MonoClass *klass);
MonoClass*      (*mono_object_get_class)(MonoObject *obj);
MonoDomain*     (*mono_object_get_domain)(MonoObject *obj);
gpointer        (*mono_object_unbox)(MonoObject *obj);

MonoArray*      (*mono_array_new)(MonoDomain *domain, MonoClass *eclass, mono_array_size_t n);
char*           (*mono_array_addr_with_size)(MonoArray *array, int size, uintptr_t idx);

MonoString*     (*mono_string_new)(MonoDomain *domain, const char *text);
MonoString*     (*mono_string_new_utf16)(MonoDomain *domain, const guint16 *text, gint32 len);
MonoString*     (*mono_string_new_len)(MonoDomain *domain, const char *text, guint length);
char*           (*mono_string_to_utf8)(MonoString *string_obj);
gunichar2*      (*mono_string_to_utf16)(MonoString *string_obj);

guint32         (*mono_gchandle_new)(MonoObject *obj, gboolean pinned);
guint32         (*mono_gchandle_new_weakref)(MonoObject *obj, gboolean track_resurrection);
MonoObject*     (*mono_gchandle_get_target)(guint32 gchandle);
void            (*mono_gchandle_free)(guint32 gchandle);

MonoMList*      (*mono_mlist_alloc)(MonoObject *data);
MonoObject*     (*mono_mlist_get_data)(MonoMList* list);
void            (*mono_mlist_set_data)(MonoMList* list, MonoObject *data);
int             (*mono_mlist_length)(MonoMList* list);
MonoMList*      (*mono_mlist_next)(MonoMList* list);
MonoMList*      (*mono_mlist_last)(MonoMList* list);
MonoMList*      (*mono_mlist_prepend)(MonoMList* list, MonoObject *data);
MonoMList*      (*mono_mlist_append)(MonoMList* list, MonoObject *data);
MonoMList*      (*mono_mlist_remove_item)(MonoMList* list, MonoMList *item);

MonoClass* (*mono_get_object_class)();
MonoClass* (*mono_get_byte_class)();
MonoClass* (*mono_get_void_class)();
MonoClass* (*mono_get_boolean_class)();
MonoClass* (*mono_get_sbyte_class)();
MonoClass* (*mono_get_int16_class)();
MonoClass* (*mono_get_uint16_class)();
MonoClass* (*mono_get_int32_class)();
MonoClass* (*mono_get_uint32_class)();
MonoClass* (*mono_get_intptr_class)();
MonoClass* (*mono_get_uintptr_class)();
MonoClass* (*mono_get_int64_class)();
MonoClass* (*mono_get_uint64_class)();
MonoClass* (*mono_get_single_class)();
MonoClass* (*mono_get_double_class)();
MonoClass* (*mono_get_char_class)();
MonoClass* (*mono_get_string_class)();
MonoClass* (*mono_get_enum_class)();
MonoClass* (*mono_get_array_class)();
MonoClass* (*mono_get_thread_class)();
MonoClass* (*mono_get_exception_class)();


void ImportMonoFunctions()
{
#ifdef _WIN32
    auto mono = ::GetModuleHandleA("mono.dll");
    g_mono_dll = mono;
    if (mono) {
#define Import(Name) (void*&)Name = ::GetProcAddress(mono, #Name)

        Import(g_free);

        Import(mono_domain_get);
        Import(mono_domain_assembly_open);
        Import(mono_assembly_get_image);
        Import(mono_assembly_fill_assembly_name);
        Import(mono_image_get_assembly);
        Import(mono_stringify_assembly_name);

        Import(mono_thread_current);
        Import(mono_thread_attach);
        Import(mono_thread_detach);
        Import(mono_thread_suspend_all_other_threads);
        Import(mono_thread_abort_all_other_threads);
        Import(mono_jit_thread_attach);

        Import(mono_add_internal_call);
        Import(mono_runtime_invoke);

        Import(mono_type_get_name);
        Import(mono_type_get_class);

        Import(mono_class_from_name);
        Import(mono_class_get_name);
        Import(mono_class_get_namespace);
        Import(mono_class_get_type);
        Import(mono_class_get_method_from_name);
        Import(mono_class_get_field_from_name);
        Import(mono_class_inflate_generic_method);
        Import(mono_class_is_subclass_of);
        Import(mono_class_get_property_from_name);
        Import(mono_class_get_methods);
        Import(mono_class_get_fields);
        Import(mono_class_get_properties);
        Import(mono_class_get_parent);
        Import(mono_class_get_image);

        Import(mono_method_get_name);
        Import(mono_method_signature);
        Import(mono_signature_get_param_count);
        Import(mono_signature_get_params);

        Import(mono_field_get_name);
        Import(mono_field_get_type);
        Import(mono_field_get_parent);
        Import(mono_field_get_flags);
        Import(mono_field_get_offset);
        Import(mono_field_get_data);
        Import(mono_field_get_value);
        Import(mono_field_set_value);

        Import(mono_property_get_name);
        Import(mono_property_get_set_method);
        Import(mono_property_get_get_method);
        Import(mono_property_get_parent);

        Import(mono_object_new);
        Import(mono_object_get_class);
        Import(mono_object_get_domain);
        Import(mono_object_unbox);

        Import(mono_array_new);
        Import(mono_array_addr_with_size);

        Import(mono_string_new);
        Import(mono_string_new_utf16);
        Import(mono_string_new_len);
        Import(mono_string_to_utf8);
        Import(mono_string_to_utf16);

        Import(mono_gchandle_new);
        Import(mono_gchandle_new_weakref);
        Import(mono_gchandle_get_target);
        Import(mono_gchandle_free);

        Import(mono_mlist_alloc);
        Import(mono_mlist_get_data);
        Import(mono_mlist_set_data);
        Import(mono_mlist_length);
        Import(mono_mlist_next);
        Import(mono_mlist_last);
        Import(mono_mlist_prepend);
        Import(mono_mlist_append);
        Import(mono_mlist_remove_item);

        Import(mono_get_object_class);
        Import(mono_get_byte_class);
        Import(mono_get_void_class);
        Import(mono_get_boolean_class);
        Import(mono_get_sbyte_class);
        Import(mono_get_int16_class);
        Import(mono_get_uint16_class);
        Import(mono_get_int32_class);
        Import(mono_get_uint32_class);
        Import(mono_get_intptr_class);
        Import(mono_get_uintptr_class);
        Import(mono_get_int64_class);
        Import(mono_get_uint64_class);
        Import(mono_get_single_class);
        Import(mono_get_double_class);
        Import(mono_get_char_class);
        Import(mono_get_string_class);
        Import(mono_get_enum_class);
        Import(mono_get_array_class);
        Import(mono_get_thread_class);
        Import(mono_get_exception_class);
#undef Import
    }
#else
    // todo
#endif
}

static struct _ImportMonoFunctions {
    _ImportMonoFunctions() { ImportMonoFunctions(); }
} g_ImportMonoFunctions;

