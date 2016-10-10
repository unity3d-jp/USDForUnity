#include "pch.h"
#include "Mono.h"
#include "MonoWrapper.h"

mImage mImage::findImage(const char *name)
{
    return mono_assembly_get_image(mono_domain_assembly_open(mono_domain_get(), name));
}

mClass mImage::findClass(const char *namespace_, const char *class_name)
{
    return mono_class_from_name(m_rep, namespace_, class_name);
}


const char* mType::getName() const
{
    if (!m_rep) { return nullptr; }
    return mono_type_get_name(m_rep);
}

mClass mType::getClass() const
{
    return mono_type_get_class(m_rep);
}



const char* mField::getName() const
{
    if (!m_rep) { return nullptr; }
    return mono_field_get_name(m_rep);
}

mType mField::getType() const
{
    return mono_field_get_type(m_rep);
}

int mField::getOffset() const
{
    return mono_field_get_offset(m_rep);
}

void mField::getValueImpl(mObject obj, void *p) const
{
    if (!m_rep) { return; }
    mono_field_get_value(obj.get(), m_rep, p);
}

void mField::setValueImpl(mObject obj, const void *p)
{
    if (!m_rep) { return; }
    mono_field_set_value(obj.get(), m_rep, (void*)p);
}


const char* mProperty::getName() const
{
    if (!m_rep) { return nullptr; }
    return mono_property_get_name(m_rep);
}

mMethod mProperty::getGetter() const
{
    if (!m_rep) { return nullptr; }
    return mono_property_get_get_method(m_rep);
}

mMethod mProperty::getSetter() const
{
    if (!m_rep) { return nullptr; }
    return mono_property_get_set_method(m_rep);
}



#define METHOD_ATTRIBUTE_MEMBER_ACCESS_MASK        0x0007
#define METHOD_ATTRIBUTE_COMPILER_CONTROLLED       0x0000
#define METHOD_ATTRIBUTE_PRIVATE                   0x0001
#define METHOD_ATTRIBUTE_FAM_AND_ASSEM             0x0002
#define METHOD_ATTRIBUTE_ASSEM                     0x0003
#define METHOD_ATTRIBUTE_FAMILY                    0x0004
#define METHOD_ATTRIBUTE_FAM_OR_ASSEM              0x0005
#define METHOD_ATTRIBUTE_PUBLIC                    0x0006
#define METHOD_ATTRIBUTE_STATIC                    0x0010
#define METHOD_ATTRIBUTE_FINAL                     0x0020
#define METHOD_ATTRIBUTE_VIRTUAL                   0x0040
#define METHOD_ATTRIBUTE_HIDE_BY_SIG               0x0080
#define METHOD_ATTRIBUTE_VTABLE_LAYOUT_MASK        0x0100
#define METHOD_ATTRIBUTE_REUSE_SLOT                0x0000
#define METHOD_ATTRIBUTE_NEW_SLOT                  0x0100
#define METHOD_ATTRIBUTE_STRICT                    0x0200
#define METHOD_ATTRIBUTE_ABSTRACT                  0x0400
#define METHOD_ATTRIBUTE_SPECIAL_NAME              0x0800
#define METHOD_ATTRIBUTE_PINVOKE_IMPL              0x2000
#define METHOD_ATTRIBUTE_UNMANAGED_EXPORT          0x0008


const char* mMethod::getName() const
{
    if (!m_rep) { return nullptr; }
    return mono_method_get_name(m_rep);
}


mObject mMethod::invoke(mObject obj, void **args)
{
    if (!m_rep) { return nullptr; }
    return mono_runtime_invoke(m_rep, obj.get(), args, nullptr);
}

mMethod mMethod::instantiate(mClass *params, size_t nparams, void *& mem)
{
    if (mem == nullptr) {
        mem = malloc(sizeof(MonoGenericInst) + (sizeof(void*)*(nparams - 1)));
    }
    MonoGenericInst *gi = (MonoGenericInst*)mem;
    gi->id = -1;
    gi->is_open = 0; // must be zero!
    gi->type_argc = nparams;
    for (size_t i = 0; i < nparams; ++i) {
        gi->type_argv[i++] = params[i].getType().get();
    }
    MonoGenericContext ctx = { nullptr, gi };
    return mono_class_inflate_generic_method(m_rep, &ctx);
}


const char* mClass::getName() const
{
    if (!m_rep) { return nullptr; }
    return mono_class_get_name(m_rep);
}

mType mClass::getType() const
{
    if (!m_rep) { return nullptr; }
    return mono_class_get_type(m_rep);
}

mField mClass::findField(const char *name) const
{
    if (!m_rep) { return nullptr; }
    return mono_class_get_field_from_name(m_rep, name);
}

mProperty mClass::findProperty(const char *name) const
{
    if (!m_rep) { return nullptr; }
    return mono_class_get_property_from_name(m_rep, name);
}

mMethod mClass::findMethod(const char *name, int num_args, const char **typenames) const
{
    if (!m_rep) { return nullptr; }

    if (typenames) {
        for (mClass mc = m_rep; mc; mc = mc.getParent()) {
            MonoMethod *method;
            gpointer iter = nullptr;
            while ((method = mono_class_get_methods(m_rep, &iter))) {
                if (strcmp(mono_method_get_name(method), name) != 0) { continue; }

                MonoMethodSignature *sig = mono_method_signature(method);
                if (mono_signature_get_param_count(sig) != num_args) { continue; }

                MonoType *mt = nullptr;
                gpointer iter = nullptr;
                bool match = true;
                for (int i = 0; i < num_args; ++i) {
                    mt = mono_signature_get_params(sig, &iter);
                    if (strcmp(mono_type_get_name(mt), typenames[i]) != 0) {
                        match = false;
                        break;
                    }
                }
                if (match) { return method; }
            }
        }
    }
    else {
        for (mClass mc = m_rep; mc; mc = mc.getParent()) {
            if (MonoMethod *ret = mono_class_get_method_from_name(mc.m_rep, name, num_args)) {
                return ret;
            }
        }
    }

    return nullptr;
}

void mClass::eachFields(const std::function<void(mField&)> &f)
{
    MonoClassField *field = nullptr;
    gpointer iter = nullptr;
    while ((field = mono_class_get_fields(m_rep, &iter))) {
        mField mf = field;
        f(mf);
    }
}

void mClass::eachProperties(const std::function<void(mProperty&)> &f)
{
    MonoProperty *prop = nullptr;
    gpointer iter = nullptr;
    while ((prop = mono_class_get_properties(m_rep, &iter))) {
        mProperty mp = prop;
        f(mp);
    }
}

void mClass::eachMethods(const std::function<void(mMethod&)> &f)
{
    MonoMethod *method = nullptr;
    gpointer iter = nullptr;
    while ((method = mono_class_get_methods(m_rep, &iter))) {
        mMethod mm = method;
        f(mm);
    }
}

void mClass::eachFieldsUpwards(const std::function<void(mField&, mClass&)> &f)
{
    mClass c = m_rep;
    do {
        MonoClassField *field = nullptr;
        gpointer iter = nullptr;
        while (field = mono_class_get_fields(c.m_rep, &iter)) {
            mField m = field;
            f(m, c);
        }
        c = c.getParent();
    } while (c);
}

void mClass::eachPropertiesUpwards(const std::function<void(mProperty&, mClass&)> &f)
{
    mClass c = m_rep;
    do {
        MonoProperty *prop = nullptr;
        gpointer iter = nullptr;
        while (prop = mono_class_get_properties(c.m_rep, &iter)) {
            mProperty m = prop;
            f(m, c);
        }
        c = c.getParent();
    } while (c);
}

void mClass::eachMethodsUpwards(const std::function<void(mMethod&, mClass&)> &f)
{
    mClass c = m_rep;
    do {
        MonoMethod *method = nullptr;
        gpointer iter = nullptr;
        while (method = mono_class_get_methods(c.m_rep, &iter)) {
            mMethod m = method;
            f(m, c);
        }
        c = c.getParent();
    } while (c);
}


mClass mClass::getParent() const
{
    return mono_class_get_parent(m_rep);
}


mClass mObject::getClass() const
{
    return mono_object_get_class(m_rep);
}

MonoDomain* mObject::getDomain() const
{
    return mono_object_get_domain(m_rep);
}

/*static*/ mObject mObject::New(mClass mclass)
{
    return mono_object_new(mono_domain_get(), mclass.get());
}

bool mObject::isNull() const { return !m_rep || !*(void**)(m_rep + 1); }

void* mObject::unbox() { return m_rep + 1; }
void* mObject::unboxValue() { return m_rep; }

mField mObject::findField(const char *name) const
{
    return getClass().findField(name);
}
mProperty mObject::findProperty(const char *name) const
{
    return getClass().findProperty(name);
}
mMethod mObject::findMethod(const char *name, int num_args, const char **typenames) const
{
    return getClass().findMethod(name, num_args);
}

mObject mObject::invoke(mMethod method)
{
    return mono_runtime_invoke(method.get(), m_rep, nullptr, nullptr);
}
mObject mObject::invoke(mMethod method, void *a0)
{
    void *args[] = { a0 };
    return mono_runtime_invoke(method.get(), m_rep, args, nullptr);
}
mObject mObject::invoke(mMethod method, void *a0, void *a1)
{
    void *args[] = { a0, a1 };
    return mono_runtime_invoke(method.get(), m_rep, args, nullptr);
}
mObject mObject::invoke(mMethod method, void *a0, void *a1, void *a2)
{
    void *args[] = { a0, a1, a2 };
    return mono_runtime_invoke(method.get(), m_rep, args, nullptr);
}


struct cpsLessCString
{
    bool operator()(const char *a, const char *b) const
    {
        return strcmp(a,b)<0;
    }
};

class cpsMethodManager
{
public:
    typedef std::map<const char*, void*, cpsLessCString> MethodTable;
    static cpsMethodManager* getInstance();
    void addMethod(const char * name, void *method);
    void registerAll();

private:
    static cpsMethodManager *s_inst;
    MethodTable m_methods;
};

/*static*/ cpsMethodManager* cpsMethodManager::s_inst;
/*static*/ cpsMethodManager* cpsMethodManager::getInstance()
{
    if (!s_inst) {
        s_inst = new cpsMethodManager();
    }
    return s_inst;
}

void cpsMethodManager::addMethod(const char * name, void *method)
{
    m_methods[name] = method;
}

void cpsMethodManager::registerAll()
{
    for (auto m : m_methods) {
        mono_add_internal_call(m.first, m.second);
    }
}


bool mIsSubclassOf(mClass parent, mClass child)
{
    return mono_class_is_subclass_of(parent.get(), child.get(), false) != 0;
}

void mAddMethod(const char *name, void *addr)
{
    mono_add_internal_call(name, addr);
}

/*static*/ mString mString::New(const mchar8 *str, int len)
{
    return mString(mono_string_new_len(mono_domain_get(), str, len == -1 ? (int)strlen(str) : len));
}

/*static*/ mString mString::New(const mchar16 *str, int len)
{
    if (len == -1) {
        for (len = 0;; ++len) {
            if (str[len] == 0) { break; }
        }
    }
    return mString(mono_string_new_utf16(mono_domain_get(), str, len));
}

size_t mString::size() const
{
    return ((MonoString*)m_rep)->length;
}

const mchar8* mString::toUTF8()
{
    return mono_string_to_utf8((MonoString*)m_rep);
}

const mchar16* mString::toUTF16()
{
    return mono_string_to_utf16((MonoString*)m_rep);
}


/*static*/ mArray mArray::New(mClass klass, size_t size)
{
    MonoArray *ret = mono_array_new(mono_domain_get(), klass.get(), (mono_array_size_t)size);
    return ret;
}

size_t mArray::size() const
{
    return ((MonoArray*)m_rep)->max_length;
}
void* mArray::data()
{
    return ((MonoArray*)m_rep)->vector;
}

#define mDefBuiltinType(Type, ClassGetter, MonoTypename)\
    mClass& Type::_getClass() { static mClass& s_class=mCreateClassCache(ClassGetter); return s_class; }\
    const char* Type::_getTypename() { return MonoTypename; }\
    const char* Type::_getTypenameRef() { return MonoTypename "&"; }\
    const char* Type::_getTypenameArray() { return MonoTypename "[]"; }

mDefBuiltinType(mVoid,   mono_get_void_class,   "System.Void"   );
mDefBuiltinType(mIntPtr, mono_get_intptr_class, "System.IntPtr" );
mDefBuiltinType(mBool,   mono_get_boolean_class,"System.Boolean");
mDefBuiltinType(mByte,   mono_get_byte_class,   "System.Byte"   );
mDefBuiltinType(mInt32,  mono_get_int32_class,  "System.Int32"  );
mDefBuiltinType(mEnum,   mono_get_enum_class,   "System.Enum"   );
mDefBuiltinType(mSingle, mono_get_single_class, "System.Single" );
mDefBuiltinType(mDouble, mono_get_double_class, "System.Double" );
mDefBuiltinType(mObject, mono_get_object_class, "System.Object" );
mDefBuiltinType(mString, mono_get_string_class, "System.String" );
#undef mDefBuiltinType

uint32_t mPin(mObject obj)
{
    return mono_gchandle_new(obj.get(), 1);
}

void mUnpin(uint32_t handle)
{
    mono_gchandle_free(handle);
}



// cache manager

class mICache
{
public:
    virtual ~mICache() {}
    virtual void clear() = 0;
    virtual void rebind() = 0;
};

class mImageCache : public mImage, public mICache
{
public:
    mImageCache(const char *name);
    void clear();
    void rebind();

private:
    const char *m_name;
};

class mClassCache : public mClass, public mICache
{
public:
    typedef MonoClass* (*Initializer)();

    mClassCache(mImage& img, const char *ns, const char *name);
    mClassCache(Initializer init);
    void clear() override;
    void rebind() override;

private:
    mImage *m_image = nullptr;
    const char *m_name = nullptr;
    const char *m_namespace = nullptr;
    Initializer m_initializer = nullptr;
};

class mFieldCache : public mField, public mICache
{
public:
    mFieldCache(mClass& mclass, const char *name);
    void clear() override;
    void rebind() override;

private:
    mClass *m_class;
    const char *m_name;
};

class mMethodCache : public mMethod, public mICache
{
public:
    mMethodCache(mClass& mclass, const char *name, int nargs = -1);
    mMethodCache(mClass& mclass, const char *name, std::vector<const char*> arg_types);
    void clear() override;
    void rebind() override;

private:
    mClass *m_class;
    const char *m_name;
    int m_num_args = -1;
    std::vector<const char*> m_typenames;
};

class mIMethodCache : public mMethod, public mICache
{
public:
    mIMethodCache(mMethod& generics, std::vector<mClass*>& param);
    template<class T> mIMethodCache(mMethod& generics) { mIMethodCache(generics, mTypeof<T>()); }
    void clear() override;
    void rebind() override;

private:
    mMethod *m_generics;
    std::vector<mClass*> m_params;
    void *m_mem = nullptr;
};

class mPropertyCache : public mProperty, public mICache
{
public:
    mPropertyCache(mClass& mclass, const char *name);
    void clear() override;
    void rebind() override;

private:
    mClass *m_class;
    const char *m_name;
};

static std::vector<mICache*> g_mCaches;
static std::mutex g_mCache_mutex;

static void mRegisterCache(mICache *v)
{
    v->rebind();

    std::unique_lock<std::mutex> l(g_mCache_mutex);
    g_mCaches.push_back(v);
}

void mClearCache()
{
    std::unique_lock<std::mutex> l(g_mCache_mutex);
    for (auto o : g_mCaches) { o->clear(); }
}

void mRebindCache()
{
    std::unique_lock<std::mutex> l(g_mCache_mutex);
    for (auto o : g_mCaches) { o->rebind(); }
}


mImageCache::mImageCache(const char *name)
    : mImage(nullptr)
    , m_name(name)
{
    mRegisterCache(this);
}
void mImageCache::clear() { m_rep = nullptr; }
void mImageCache::rebind() { m_rep = findImage(m_name).get(); }


mClassCache::mClassCache(mImage& img, const char *ns, const char *name)
    : mClass(nullptr)
    , m_image(&img)
    , m_namespace(ns)
    , m_name(name)
{
    mRegisterCache(this);
}

mClassCache::mClassCache(Initializer init)
    : mClass(nullptr)
    , m_initializer(init)
{
}

void mClassCache::clear() { m_rep = nullptr; }
void mClassCache::rebind() {
    m_rep = m_initializer ? m_initializer() : m_image->findClass(m_namespace, m_name).get();
}


mFieldCache::mFieldCache(mClass& mclass, const char *name)
    : mField(nullptr)
    , m_class(&mclass)
    , m_name(name)
{
    mRegisterCache(this);
}
void mFieldCache::clear() { m_rep = nullptr; }
void mFieldCache::rebind() { m_rep = m_class->findField(m_name).get(); }


mMethodCache::mMethodCache(mClass& mclass, const char *name, int nargs)
    : mMethod(nullptr)
    , m_class(&mclass)
    , m_name(name)
    , m_num_args(nargs)
{
    mRegisterCache(this);
}
mMethodCache::mMethodCache(mClass& mclass, const char *name, std::vector<const char*> typenames)
    : mMethod(nullptr)
    , m_class(&mclass)
    , m_name(name)
    , m_typenames(typenames)
{
    mRegisterCache(this);
}
void mMethodCache::clear() { m_rep = nullptr; }
void mMethodCache::rebind() {
    if (m_typenames.empty()) {
        m_rep = m_class->findMethod(m_name, m_num_args).get();
    }
    else {
        m_rep = m_class->findMethod(m_name, (int)m_typenames.size(), m_typenames.data()).get();
    }
}

mIMethodCache::mIMethodCache(mMethod& generics, std::vector<mClass*>& param)
    : mMethod(nullptr)
    , m_generics(&generics)
    , m_params(param)
{
    mRegisterCache(this);
}
void mIMethodCache::clear() { m_rep = nullptr; }
void mIMethodCache::rebind() {
    std::vector<mClass> params;
    for (auto *c : m_params) { params.push_back(*c); }
    m_rep = m_generics->instantiate(params.data(), params.size(), m_mem).get();
}

mPropertyCache::mPropertyCache(mClass& mclass, const char *name)
    : mProperty(nullptr)
    , m_class(&mclass)
    , m_name(name)
{
    mRegisterCache(this);
}
void mPropertyCache::clear() { m_rep = nullptr; }
void mPropertyCache::rebind() { m_rep = m_class->findProperty(m_name).get(); }


mImage& mCreateImageCache(const char *name)
{
    return *new mImageCache(name);
}
mClass& mCreateClassCache(mImage& img, const char *ns, const char *name)
{
    return *new mClassCache(img, ns, name);
}
mClass& mCreateClassCache(MonoClass* (*initializer)())
{
    return *new mClassCache(initializer);
}
mField& mCreateFieldCache(mClass& mclass, const char *name)
{
    return *new mFieldCache(mclass, name);
}
mMethod& mCreateMethodCache(mClass& mclass, const char *name, int nargs)
{
    return *new mMethodCache(mclass, name, nargs);
}
mMethod& mCreateMethodCache(mClass& mclass, const char *name, std::vector<const char*> typenames)
{
    return *new mMethodCache(mclass, name, typenames);
}
mMethod& mCreateMethodCache(mMethod& generics, std::vector<mClass*> params)
{
    return *new mIMethodCache(generics, params);
}
mProperty& mCreatePropertyCache(mClass& mclass, const char *name)
{
    return *new mPropertyCache(mclass, name);
}
