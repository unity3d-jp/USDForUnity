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
struct MonoObject;
struct MonoArray;
struct MonoString;

#define MONO_ZERO_LEN_ARRAY 1

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

extern MonoClass*       (*mono_class_from_name)(MonoImage *image, const char *namespaceString, const char *classnameString);
extern MonoType*        (*mono_class_get_type)(MonoClass *klass);
extern MonoMethod*      (*mono_class_get_method_from_name)(MonoClass *klass, const char *name, int param_count);
extern MonoClassField*  (*mono_class_get_field_from_name)(MonoClass *klass, const char *name);
extern MonoMethod*      (*mono_class_inflate_generic_method)(MonoMethod *method, MonoGenericContext *context);

extern guint32          (*mono_field_get_offset)(MonoClassField *field);

extern MonoObject*      (*mono_object_new)(MonoDomain *domain, MonoClass *klass);
extern MonoClass*       (*mono_object_get_class)(MonoObject *obj);
extern gpointer         (*mono_object_unbox)(MonoObject *obj);

extern MonoArray*       (*mono_array_new)(MonoDomain *domain, MonoClass *eclass, mono_array_size_t n);
extern char*            (*mono_array_addr_with_size)(MonoArray *array, int size, uintptr_t idx);

extern MonoString*      (*mono_string_new)(MonoDomain *domain, const char *text);
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




// helper

template<class T> static inline T* Unbox(MonoObject *mobj)
{
    return ((T**)mobj)[2];
}

template<class T> struct UnboxValueImpl;
template<class T> struct UnboxValueImpl<T*> { T* operator()(MonoObject *mobj) { return (T*)mobj; } };
template<class T> struct UnboxValueImpl<T&> { T& operator()(MonoObject *mobj) { return *((T*)mobj); } };
template<class T> static inline T UnboxValue(MonoObject *mobj) { return UnboxValueImpl<T>()(mobj); }

template<class T> struct MFieldImpl;
template<class T> struct MFieldImpl<T*> { T* operator()(MonoObject *mobj, int pos) { return *(T**)((size_t)mobj+pos); } };
template<class T> struct MFieldImpl<T&> { T& operator()(MonoObject *mobj, int pos) { return *((T*)((size_t)mobj + pos)); } };
template<class T> static inline T MField(MonoObject *mobj, int pos) { return pos == 0 ? nullptr : MFieldImpl<T>()(mobj, pos); }

inline MonoObject* MCall(MonoObject *self, MonoMethod *method)
{
    return mono_runtime_invoke(method, self, nullptr, nullptr);
}
inline MonoObject* MCall(MonoObject *self, MonoMethod *method, void *a0)
{
    void *args[] = { a0 };
    return mono_runtime_invoke(method, self, args, nullptr);
}
inline MonoObject* MCall(MonoObject *self, MonoMethod *method, void *a0, void *a1)
{
    void *args[] = { a0, a1 };
    return mono_runtime_invoke(method, self, args, nullptr);
}
inline MonoObject* MCall(MonoObject *self, MonoMethod *method, void *a0, void *a1, void *a2)
{
    void *args[] = { a0, a1, a2 };
    return mono_runtime_invoke(method, self, args, nullptr);
}

#define MM(N) mono_class_get_method_from_name(mono_object_get_class(self), method, N)
inline MonoObject* MCall(MonoObject *self, const char *method)
{
    return MCall(self, MM(0));
}
inline MonoObject* MCall(MonoObject *self, const char *method, void *a0)
{
    return MCall(self, MM(1), a0);
}
inline MonoObject* MCall(MonoObject *self, const char *method, void *a0, void *a1)
{
    return MCall(self, MM(2), a0, a1);
}
inline MonoObject* MCall(MonoObject *self, const char *method, void *a0, void *a1, void *a2)
{
    return MCall(self, MM(3), a0, a1, a2);
}
#undef MM



class MObject
{
public:
    MObject(const MObject& o) = delete;
    MObject& operator=(const MObject& o) = delete;

    MObject();
    ~MObject();

    void allocate(MonoClass *mc);
    void rerlease();
    MonoObject* get();
    operator bool() const;

private:
    MonoObject *m_rep = nullptr;
    guint32 m_gch = 0;
};

class MArray
{
public:
    MArray(const MArray& o) = delete;
    MArray& operator=(const MArray& o) = delete;

    MArray();
    ~MArray();

    void allocate(MonoClass *mc, size_t size);
    void release();

    size_t size() const;
    void* data();
    const void* data() const;
    MonoArray* get();
    operator bool() const;

private:
    MonoArray *m_rep = nullptr;
    size_t m_size = 0;
    guint32 m_gch = 0;
};
