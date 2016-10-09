#pragma once

typedef int            gboolean;
typedef int            gint;
typedef unsigned int   guint;
typedef short          gshort;
typedef unsigned short gushort;
typedef long           glong;
typedef unsigned long  gulong;
typedef void *         gpointer;
typedef const void *   gconstpointer;
typedef char           gchar;
typedef unsigned char  guchar;

typedef __int8              gint8;
typedef unsigned __int8     guint8;
typedef __int16             gint16;
typedef unsigned __int16    guint16;
typedef __int32             gint32;
typedef unsigned __int32    guint32;
typedef __int64             gint64;
typedef unsigned __int64    guint64;
typedef float               gfloat;
typedef double              gdouble;
typedef unsigned __int16    gunichar2;

typedef guint32 mono_array_size_t;
typedef gint32 mono_array_lower_bound_t;

struct MonoDomain;
struct MonoAssembly;
struct MonoImage;
struct MonoThread;

struct MonoType;
struct MonoClass;
struct MonoClassField;
struct MonoMethod;
struct MonoMethodSignature;
struct MonoProperty;
struct MonoObject;
struct MonoArray;
struct MonoString;
struct MonoVTable;
struct MonoThreadsSync;

#define MONO_ZERO_LEN_ARRAY 1

struct MonoObject {
    MonoVTable *vtable;
    MonoThreadsSync *synchronisation;
};

struct MonoArrayBounds {
    mono_array_size_t length;
    mono_array_lower_bound_t lower_bound;
};

struct MonoArray {
    MonoObject obj;
    MonoArrayBounds *bounds;
    mono_array_size_t max_length;
    double vector[MONO_ZERO_LEN_ARRAY];
};

struct MonoString {
    MonoObject object;
    gint32 length;
    gunichar2 chars[MONO_ZERO_LEN_ARRAY];
};

struct MonoGenericInst {
    guint id;
    guint type_argc : 22;
    guint is_open : 1;
    MonoType *type_argv[MONO_ZERO_LEN_ARRAY];
};

struct MonoGenericContext {
    MonoGenericInst *class_inst;
    MonoGenericInst *method_inst;
};



extern void *g_mono_dll;

extern MonoDomain*      (*mono_domain_get)(void);
extern MonoAssembly*    (*mono_domain_assembly_open)(MonoDomain *domain, const char *assemblyName);
extern MonoImage*       (*mono_assembly_get_image)(MonoAssembly *assembly);

extern MonoThread*      (*mono_thread_current)(void);
extern MonoThread*      (*mono_thread_attach)(MonoDomain *domain);
extern void             (*mono_thread_detach)(MonoThread *thread);
extern void             (*mono_thread_suspend_all_other_threads)();
extern void             (*mono_thread_abort_all_other_threads)();
extern void             (*mono_jit_thread_attach)(MonoDomain *domain);

extern void             (*mono_add_internal_call)(const char *name, void *method);
extern MonoObject*      (*mono_runtime_invoke)(MonoMethod *method, MonoObject *obj, void **params, void **exc);

extern char*            (*mono_type_get_name)(MonoType *type);
extern MonoClass*       (*mono_type_get_class)(MonoType *type);

extern MonoClass*       (*mono_class_from_name)(MonoImage *image, const char *namespaceString, const char *classnameString);
extern const char*      (*mono_class_get_name)(MonoClass *klass);
extern MonoType*        (*mono_class_get_type)(MonoClass *klass);
extern MonoMethod*      (*mono_class_get_method_from_name)(MonoClass *klass, const char *name, int param_count);
extern MonoClassField*  (*mono_class_get_field_from_name)(MonoClass *klass, const char *name);
extern MonoMethod*      (*mono_class_inflate_generic_method)(MonoMethod *method, MonoGenericContext *context);
extern gboolean         (*mono_class_is_subclass_of)(MonoClass *klass, MonoClass *klassc, gboolean check_interfaces);
extern MonoProperty*    (*mono_class_get_property_from_name)(MonoClass *klass, const char *name);
extern MonoMethod*      (*mono_class_get_methods)(MonoClass* klass, gpointer *iter);
extern MonoClassField*  (*mono_class_get_fields)(MonoClass* klass, gpointer *iter);
extern MonoProperty*    (*mono_class_get_properties)(MonoClass* klass, gpointer *iter);
extern MonoClass*       (*mono_class_get_parent)(MonoClass *klass);


extern const char*      (*mono_method_get_name)(MonoMethod *method);
extern MonoMethodSignature* (*mono_method_signature)(MonoMethod *method);
extern guint32          (*mono_signature_get_param_count)(MonoMethodSignature *sig);
extern MonoType*        (*mono_signature_get_params)(MonoMethodSignature *sig, gpointer *iter);

extern const char*      (*mono_field_get_name)(MonoClassField *field);
extern MonoType*        (*mono_field_get_type)(MonoClassField *field);
extern MonoClass*       (*mono_field_get_parent)(MonoClassField *field);
extern guint32          (*mono_field_get_flags)(MonoClassField *field);
extern guint32          (*mono_field_get_offset)(MonoClassField *field);
extern const char*      (*mono_field_get_data)(MonoClassField *field);
extern void             (*mono_field_get_value)(MonoObject *obj, MonoClassField *field, void *value);
extern void             (*mono_field_set_value)(MonoObject *obj, MonoClassField *field, void *value);

extern const char*      (*mono_property_get_name)(MonoProperty *prop);
extern MonoMethod*      (*mono_property_get_set_method)(MonoProperty *prop);
extern MonoMethod*      (*mono_property_get_get_method)(MonoProperty *prop);
extern MonoClass*       (*mono_property_get_parent)(MonoProperty *prop);

extern MonoObject*      (*mono_object_new)(MonoDomain *domain, MonoClass *klass);
extern MonoClass*       (*mono_object_get_class)(MonoObject *obj);
extern MonoDomain*      (*mono_object_get_domain)(MonoObject *obj);
extern gpointer         (*mono_object_unbox)(MonoObject *obj);

extern MonoArray*       (*mono_array_new)(MonoDomain *domain, MonoClass *eclass, mono_array_size_t n);
extern char*            (*mono_array_addr_with_size)(MonoArray *array, int size, uintptr_t idx);

extern MonoString*      (*mono_string_new)(MonoDomain *domain, const char *text);
extern MonoString*      (*mono_string_new_utf16)(MonoDomain *domain, const guint16 *text, gint32 len);
extern MonoString*      (*mono_string_new_len)(MonoDomain *domain, const char *text, guint length);
extern char*            (*mono_string_to_utf8)(MonoString *string_obj);
extern gunichar2*       (*mono_string_to_utf16)(MonoString *string_obj);

extern guint32          (*mono_gchandle_new)(MonoObject *obj, gboolean pinned);
extern void             (*mono_gchandle_free)(guint32 gchandle);

extern MonoClass* (*mono_get_object_class)();
extern MonoClass* (*mono_get_byte_class)();
extern MonoClass* (*mono_get_void_class)();
extern MonoClass* (*mono_get_boolean_class)();
extern MonoClass* (*mono_get_sbyte_class)();
extern MonoClass* (*mono_get_int16_class)();
extern MonoClass* (*mono_get_uint16_class)();
extern MonoClass* (*mono_get_int32_class)();
extern MonoClass* (*mono_get_uint32_class)();
extern MonoClass* (*mono_get_intptr_class)();
extern MonoClass* (*mono_get_uintptr_class)();
extern MonoClass* (*mono_get_int64_class)();
extern MonoClass* (*mono_get_uint64_class)();
extern MonoClass* (*mono_get_single_class)();
extern MonoClass* (*mono_get_double_class)();
extern MonoClass* (*mono_get_char_class)();
extern MonoClass* (*mono_get_string_class)();
extern MonoClass* (*mono_get_enum_class)();
extern MonoClass* (*mono_get_array_class)();
extern MonoClass* (*mono_get_thread_class)();
extern MonoClass* (*mono_get_exception_class)();

