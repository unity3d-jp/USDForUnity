#pragma once

#ifdef usdiEnableMonoBinding
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

typedef int8_t      gint8;
typedef uint8_t     guint8;
typedef int16_t     gint16;
typedef uint16_t    guint16;
typedef int32_t     gint32;
typedef uint32_t    guint32;
typedef int64_t     gint64;
typedef uint64_t    guint64;
typedef float       gfloat;
typedef double      gdouble;
typedef guint16     gunichar2;

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
struct MonoMList;

struct GSList {
    gpointer data;
    GSList *next;
};

#define MONO_PUBLIC_KEY_TOKEN_LENGTH	17

struct MonoAssemblyName
{
    const char *name;
    const char *culture;
    const char *hash_value;
    const guint8* public_key;
    guchar public_key_token[MONO_PUBLIC_KEY_TOKEN_LENGTH];
    guint32 hash_alg;
    guint32 hash_len;
    guint32 flags;
    guint16 major, minor, build, revision;
};

struct MonoAssembly
{
    int ref_count;
    char *basedir;
    MonoAssemblyName aname;
    MonoImage *image;
    GSList *friend_assembly_names;
    guint8 friend_assembly_names_inited;
    guint8 in_gac;
    guint8 dynamic;
    guint8 corlib_internal;
    gboolean ref_only;
    guint32 ecma : 2;
    guint32 aptc : 2;
    guint32 fulltrust : 2;
    guint32 unmanaged : 2;
    guint32 skipverification : 2;
};

#define MONO_ZERO_LEN_ARRAY 1

#if defined(_MSC_VER) && defined(PLATFORM_IPHONE_XCOMP)
#   define USE_UINT8_BIT_FIELD(type, field) guint8 field 
#else
#   define USE_UINT8_BIT_FIELD(type, field) type field
#endif

typedef gpointer MonoRuntimeGenericContext;

struct MonoVTable {
    MonoClass  *klass;
    void *gc_descr;
    MonoDomain *domain;
    gpointer    data;
    gpointer    type; /* System.Type type for klass */
    guint8     *interface_bitmap;
    guint16     max_interface_id;
    guint8      rank;
    USE_UINT8_BIT_FIELD(guint, remote      : 1);
    USE_UINT8_BIT_FIELD(guint, initialized : 1);
    USE_UINT8_BIT_FIELD(guint, init_failed : 1);
    guint32     imt_collisions_bitmap;
    MonoRuntimeGenericContext *runtime_generic_context;
    gpointer    vtable[MONO_ZERO_LEN_ARRAY];
};

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

struct MonoMList {
    MonoObject object;
    MonoMList *next;
    MonoObject *data;
};



extern void *g_mono_dll;

extern void (*_g_free)(void *ptr);

extern MonoDomain*      (*_mono_domain_get)(void);
extern MonoAssembly*    (*_mono_domain_assembly_open)(MonoDomain *domain, const char *assemblyName);
extern MonoImage*       (*_mono_assembly_get_image)(MonoAssembly *assembly);
extern gboolean         (*_mono_assembly_fill_assembly_name)(MonoImage *image, MonoAssemblyName *aname);
extern MonoAssembly*    (*_mono_image_get_assembly)(MonoImage *image);
extern char*            (*_mono_stringify_assembly_name)(MonoAssemblyName *aname);

extern MonoThread*      (*_mono_thread_current)(void);
extern MonoThread*      (*_mono_thread_attach)(MonoDomain *domain);
extern void             (*_mono_thread_detach)(MonoThread *thread);
extern void             (*_mono_thread_suspend_all_other_threads)();
extern void             (*_mono_thread_abort_all_other_threads)();
extern void             (*_mono_jit_thread_attach)(MonoDomain *domain);

extern void             (*_mono_add_internal_call)(const char *name, void *method);
extern MonoObject*      (*_mono_runtime_invoke)(MonoMethod *method, MonoObject *obj, void **params, void **exc);

extern char*            (*_mono_type_get_name)(MonoType *type);
extern MonoClass*       (*_mono_type_get_class)(MonoType *type);

extern MonoClass*       (*_mono_class_from_name)(MonoImage *image, const char *namespaceString, const char *classnameString);
extern const char*      (*_mono_class_get_name)(MonoClass *klass);
extern const char*      (*_mono_class_get_namespace)(MonoClass *klass);
extern MonoType*        (*_mono_class_get_type)(MonoClass *klass);
extern MonoMethod*      (*_mono_class_get_method_from_name)(MonoClass *klass, const char *name, int param_count);
extern MonoClassField*  (*_mono_class_get_field_from_name)(MonoClass *klass, const char *name);
extern MonoMethod*      (*_mono_class_inflate_generic_method)(MonoMethod *method, MonoGenericContext *context);
extern MonoMethod*      (*_mono_class_inflate_generic_method_full)(MonoMethod *method, MonoClass *klass_hint, MonoGenericContext *context);
extern gboolean         (*_mono_class_is_subclass_of)(MonoClass *klass, MonoClass *klassc, gboolean check_interfaces);
extern MonoProperty*    (*_mono_class_get_property_from_name)(MonoClass *klass, const char *name);
extern MonoMethod*      (*_mono_class_get_methods)(MonoClass* klass, gpointer *iter);
extern MonoClassField*  (*_mono_class_get_fields)(MonoClass* klass, gpointer *iter);
extern MonoProperty*    (*_mono_class_get_properties)(MonoClass* klass, gpointer *iter);
extern MonoClass*       (*_mono_class_get_parent)(MonoClass *klass);
extern MonoImage*       (*_mono_class_get_image)(MonoClass *klass);

extern const char*      (*_mono_method_get_name)(MonoMethod *method);
extern MonoClass*       (*_mono_method_get_class)(MonoMethod *method);
extern MonoMethodSignature* (*_mono_method_signature)(MonoMethod *method);
extern guint32          (*_mono_signature_get_param_count)(MonoMethodSignature *sig);
extern MonoType*        (*_mono_signature_get_params)(MonoMethodSignature *sig, gpointer *iter);

extern const char*      (*_mono_field_get_name)(MonoClassField *field);
extern MonoType*        (*_mono_field_get_type)(MonoClassField *field);
extern MonoClass*       (*_mono_field_get_parent)(MonoClassField *field);
extern guint32          (*_mono_field_get_flags)(MonoClassField *field);
extern guint32          (*_mono_field_get_offset)(MonoClassField *field);
extern const char*      (*_mono_field_get_data)(MonoClassField *field);
extern void             (*_mono_field_get_value)(MonoObject *obj, MonoClassField *field, void *value);
extern void             (*_mono_field_set_value)(MonoObject *obj, MonoClassField *field, void *value);

extern const char*      (*_mono_property_get_name)(MonoProperty *prop);
extern MonoMethod*      (*_mono_property_get_set_method)(MonoProperty *prop);
extern MonoMethod*      (*_mono_property_get_get_method)(MonoProperty *prop);
extern MonoClass*       (*_mono_property_get_parent)(MonoProperty *prop);

extern MonoObject*      (*_mono_object_new)(MonoDomain *domain, MonoClass *klass);
extern MonoClass*       (*_mono_object_get_class)(MonoObject *obj);
extern MonoDomain*      (*_mono_object_get_domain)(MonoObject *obj);
extern gpointer         (*_mono_object_unbox)(MonoObject *obj);

extern MonoArray*       (*_mono_array_new)(MonoDomain *domain, MonoClass *eclass, mono_array_size_t n);
extern char*            (*_mono_array_addr_with_size)(MonoArray *array, int size, uintptr_t idx);

extern MonoString*      (*_mono_string_new)(MonoDomain *domain, const char *text);
extern MonoString*      (*_mono_string_new_utf16)(MonoDomain *domain, const guint16 *text, gint32 len);
extern MonoString*      (*_mono_string_new_len)(MonoDomain *domain, const char *text, guint length);
extern char*            (*_mono_string_to_utf8)(MonoString *string_obj);
extern gunichar2*       (*_mono_string_to_utf16)(MonoString *string_obj);

extern guint32          (*_mono_gchandle_new)(MonoObject *obj, gboolean pinned);
extern guint32          (*_mono_gchandle_new_weakref)(MonoObject *obj, gboolean track_resurrection);
extern MonoObject*      (*_mono_gchandle_get_target)(guint32 gchandle);
extern void             (*_mono_gchandle_free)(guint32 gchandle);

extern MonoMList*       (*_mono_mlist_alloc)(MonoObject *data);
extern MonoObject*      (*_mono_mlist_get_data)(MonoMList* list);
extern void             (*_mono_mlist_set_data)(MonoMList* list, MonoObject *data);
extern int              (*_mono_mlist_length)(MonoMList* list);
extern MonoMList*       (*_mono_mlist_next)(MonoMList* list);
extern MonoMList*       (*_mono_mlist_last)(MonoMList* list);
extern MonoMList*       (*_mono_mlist_prepend)(MonoMList* list, MonoObject *data);
extern MonoMList*       (*_mono_mlist_append)(MonoMList* list, MonoObject *data);
extern MonoMList*       (*_mono_mlist_remove_item)(MonoMList* list, MonoMList *item);

extern MonoClass* (*_mono_get_object_class)();
extern MonoClass* (*_mono_get_byte_class)();
extern MonoClass* (*_mono_get_void_class)();
extern MonoClass* (*_mono_get_boolean_class)();
extern MonoClass* (*_mono_get_sbyte_class)();
extern MonoClass* (*_mono_get_int16_class)();
extern MonoClass* (*_mono_get_uint16_class)();
extern MonoClass* (*_mono_get_int32_class)();
extern MonoClass* (*_mono_get_uint32_class)();
extern MonoClass* (*_mono_get_intptr_class)();
extern MonoClass* (*_mono_get_uintptr_class)();
extern MonoClass* (*_mono_get_int64_class)();
extern MonoClass* (*_mono_get_uint64_class)();
extern MonoClass* (*_mono_get_single_class)();
extern MonoClass* (*_mono_get_double_class)();
extern MonoClass* (*_mono_get_char_class)();
extern MonoClass* (*_mono_get_string_class)();
extern MonoClass* (*_mono_get_enum_class)();
extern MonoClass* (*_mono_get_array_class)();
extern MonoClass* (*_mono_get_thread_class)();
extern MonoClass* (*_mono_get_exception_class)();

#endif // usdiEnableMonoBinding
