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

typedef void MonoDomain;
typedef void MonoAssembly;
typedef void MonoImage;
typedef void MonoThread;

typedef void MonoClass;
typedef void MonoMethod;
typedef void MonoObject;
typedef void MonoArray;
typedef void MonoString;

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

extern MonoObject*  (*mono_object_new)(MonoDomain *domain, MonoClass *klass);
extern MonoClass*   (*mono_object_get_class)(MonoObject *obj);
extern gpointer     (*mono_object_unbox)(MonoObject *obj);

extern MonoArray*   (*mono_array_new)(MonoDomain *domain, MonoClass *eclass, mono_array_size_t n);
extern char*        (*mono_array_addr_with_size)(MonoArray *array, int size, uintptr_t idx);
#define mono_array_addr(array,type,index) ((type*)(gpointer) mono_array_addr_with_size (array, sizeof (type), index))

extern MonoString*  (*mono_string_new)(MonoDomain *domain, const char *text);
extern MonoString*  (*mono_string_new_len)(MonoDomain *domain, const char *text, guint length);
extern char*        (*mono_string_to_utf8)(MonoString *string_obj);
extern gunichar2*   (*mono_string_to_utf16)(MonoString *string_obj);

extern guint32      (*mono_gchandle_new)(MonoObject *obj, gboolean pinned);
extern void         (*mono_gchandle_free)(guint32 gchandle);




// helper

template<class T> static inline T* Unbox(MonoObject *mobj)
{
    return ((T**)mobj)[2];
}

template<class T> struct UnboxValueImpl;
template<class T> struct UnboxValueImpl<T*> { T* operator()(MonoObject *mobj) { return (T*)mobj; } };
template<class T> struct UnboxValueImpl<T&> { T& operator()(MonoObject *mobj) { return *((T*)mobj); } };
template<class T> static inline T UnboxValue(MonoObject *mobj) { return UnboxValueImpl<T>()(mobj); }


inline MonoObject* MCall(MonoObject *self, MonoMethod *method)
{
    return mono_runtime_invoke(method, self, nullptr, nullptr);
}

template<class A0>
inline MonoObject* MCall(MonoObject *self, MonoMethod *method, A0 a0)
{
    MonoObject *args[] = { a0 };
    return mono_runtime_invoke(method, self, args, nullptr);
}

template<class A0, class A1>
inline MonoObject* MCall(MonoObject *self, MonoMethod *method, A0 a0, A1 a1)
{
    MonoObject *args[] = { a0, a1 };
    return mono_runtime_invoke(method, self, args, nullptr);
}

template<class A0, class A1, class A2>
inline MonoObject* MCall(MonoObject *self, MonoMethod *method, A0 a0, A1 a1, A2 a2)
{
    MonoObject *args[] = { a0, a1, a2 };
    return mono_runtime_invoke(method, self, args, nullptr);
}



class MObject
{
public:
    MObject(const MObject& o) = delete;
    MObject& operator=(const MObject& o) = delete;

    MObject();
    ~MObject();

    void allocate(MonoClass *mc);
    void free();
    MonoArray* get();

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
    void free();
    void* data();
    const void* data() const;
    MonoArray* get();

private:
    MonoArray *m_rep = nullptr;
    guint32 m_gch = 0;
};
