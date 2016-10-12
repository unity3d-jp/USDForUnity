#include "pch.h"
#include "Mono.h"
#include "MonoWrapper.h"
#include "tls.h"



mImage mDomain::findImage(const char *name)
{
    return mono_assembly_get_image(mono_domain_assembly_open(mGetDomain().get(), name));
}

std::string mAssembly::getAssemblyName() const
{
    char *aname = mono_stringify_assembly_name(&m_rep->aname);
    std::string ret = aname;
    g_free((void*)aname);
    return ret;

}

mAssembly mImage::getAssembly()
{
    return mono_image_get_assembly(m_rep);
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
    return mono_class_get_name(m_rep);
}

const char* mClass::getNamespace() const
{
    return mono_class_get_namespace(m_rep);
}

mImage mClass::getImage() const
{
    return mono_class_get_image(m_rep);
}

mType mClass::getType() const
{
    return mono_class_get_type(m_rep);
}

mField mClass::findField(const char *name) const
{
    return mono_class_get_field_from_name(m_rep, name);
}

mProperty mClass::findProperty(const char *name) const
{
    return mono_class_get_property_from_name(m_rep, name);
}

mMethod mClass::findMethod(const char *name, int num_args, const char **typenames) const
{
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
    return mono_object_new(mGetDomain().get(), mclass.get());
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

mObject mObject::sinvoke(mMethod method)
{
    return mono_runtime_invoke(method.get(), nullptr, nullptr, nullptr);
}
mObject mObject::sinvoke(mMethod method, void *a0)
{
    void *args[] = { a0 };
    return mono_runtime_invoke(method.get(), nullptr, args, nullptr);
}
mObject mObject::sinvoke(mMethod method, void *a0, void *a1)
{
    void *args[] = { a0, a1 };
    return mono_runtime_invoke(method.get(), nullptr, args, nullptr);
}
mObject mObject::sinvoke(mMethod method, void *a0, void *a1, void *a2)
{
    void *args[] = { a0, a1, a2 };
    return mono_runtime_invoke(method.get(), nullptr, args, nullptr);
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
    return mString(mono_string_new_len(mGetDomain().get(), str, len == -1 ? (int)strlen(str) : len));
}

/*static*/ mString mString::New(const mchar16 *str, int len)
{
    if (len == -1) {
        for (len = 0;; ++len) {
            if (str[len] == 0) { break; }
        }
    }
    return mString(mono_string_new_utf16(mGetDomain().get(), str, len));
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
    MonoArray *ret = mono_array_new(mGetDomain().get(), klass.get(), (mono_array_size_t)size);
    return ret;
}

size_t mArray::size() const
{
    return m_rep == nullptr ? 0 : ((MonoArray*)m_rep)->max_length;
}
void* mArray::data()
{
    return m_rep == nullptr ? nullptr : ((MonoArray*)m_rep)->vector;
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

mString mToMString(const char *s) { return mString::New(s); }
std::string mToCString(mObject v) { return mString(v.get()).toUTF8(); }


uint32_t mGCHandleAllocate(mObject obj, bool pin)
{
    return mono_gchandle_new(obj.get(), (gboolean)pin);
}

void mGCHandleFree(uint32_t handle)
{
    mono_gchandle_free(handle);
}

mObject mGCHandleGetObject(uint32_t handle)
{
    return mono_gchandle_get_target(handle);
}




// thread management

static tls<MonoThread*> g_mthreads;

void mAttachThread()
{
    if (mono_thread_current() != nullptr) { return; }

    auto *& mthread = g_mthreads.local();
    if (!mthread) {
        mthread = mono_thread_attach(mGetDomain().get());
    }
}

void mDetachThread()
{
    auto *& mthread = g_mthreads.local();
    if (mthread) {
        mono_thread_detach(mthread);
        mthread = nullptr;
    }
}

void mDetachAllThreads()
{
    g_mthreads.each([](auto *& mthread) {
        if (mthread) {
            mono_thread_detach(mthread);
            mthread = nullptr;
        }
    });
}


mDefImage(mscorlib, "mscorlib");

mObject mGetSystemType(mClass c)
{
    static mClass& s_Type = mCreateClassCache(mGetImage(mscorlib), "System", "Type");
    static mMethod& s_GetType = mCreateMethodCache(s_Type, "GetType", 1);

    char qname[1024];
    sprintf(qname, "%s.%s, %s",
        c.getNamespace(), c.getName(), c.getImage().getAssembly().getAssemblyName().c_str());

    void *args[] = { mToMString(qname).get() };
    return s_GetType.invoke(nullptr, args);
}
