#include "pch.h"
#include "Mono.h"
#include "MonoWrapper.h"

mImage mImage::findImage(const char *name)
{
    return mono_assembly_get_image(mono_domain_assembly_open(mono_domain_get(), name));
}

mClass mImage::findClass(const char *namespace_, const char *class_name)
{
    return mono_class_from_name(mimage, namespace_, class_name);
}


const char* mType::getName() const
{
    if (!mtype) { return nullptr; }
    return mono_type_get_name(mtype);
}

mClass mType::getClass() const
{
    return mono_type_get_class(mtype);
}



const char* mField::getName() const
{
    if (!mfield) { return nullptr; }
    return mono_field_get_name(mfield);
}

mType mField::getType() const
{
    return mono_field_get_type(mfield);
}

int mField::getOffset() const
{
    return mono_field_get_offset(mfield);
}

void mField::getValueImpl(mObject obj, void *p) const
{
    if (!mfield) { return; }
    mono_field_get_value(obj, mfield, p);
}

void mField::setValueImpl(mObject obj, const void *p)
{
    if (!mfield) { return; }
    mono_field_set_value(obj, mfield, (void*)p);
}


const char* mProperty::getName() const
{
    if (!mproperty) { return nullptr; }
    return mono_property_get_name(mproperty);
}

mMethod mProperty::getGetter() const
{
    if (!mproperty) { return nullptr; }
    return mono_property_get_get_method(mproperty);
}

mMethod mProperty::getSetter() const
{
    if (!mproperty) { return nullptr; }
    return mono_property_get_set_method(mproperty);
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
    if (!mmethod) { return nullptr; }
    return mono_method_get_name(mmethod);
}


mObject mMethod::invoke(mObject obj, void **args)
{
    if (!mmethod) { return nullptr; }
    return mono_runtime_invoke(mmethod, obj, args, nullptr);
}

mMethod mMethod::instantiate(mClass *params, int num_params, void *& allocated_space)
{
    allocated_space = malloc(sizeof(MonoGenericInst)+(sizeof(void*)*(num_params-1)));
    MonoGenericInst *gi = (MonoGenericInst*)allocated_space;
    gi->id = -1;
    gi->is_open = 0; // must be zero!
    gi->type_argc = num_params;
    for (int i = 0; i < num_params; ++i) {
        gi->type_argv[i] = params[i].getType().mtype;
    }
    MonoGenericContext ctx = { nullptr, gi };
    return mono_class_inflate_generic_method(mmethod, &ctx);
}


const char* mClass::getName() const
{
    if (!mclass) { return nullptr; }
    return mono_class_get_name(mclass);
}

mType mClass::getType() const
{
    if (!mclass) { return nullptr; }
    return mono_class_get_type(mclass);
}

mField mClass::findField(const char *name) const
{
    if (!mclass) { return nullptr; }
    return mono_class_get_field_from_name(mclass, name);
}

mProperty mClass::findProperty(const char *name) const
{
    if (!mclass) { return nullptr; }
    return mono_class_get_property_from_name(mclass, name);
}

mMethod mClass::findMethod(const char *name, int num_args, const char **arg_typenames) const
{
    if (!mclass) { return nullptr; }
    if (arg_typenames != nullptr) {
        for (mClass mc = mclass; mc; mc = mc.getParent()) {
            MonoMethod *method;
            gpointer iter = nullptr;
            while ((method = mono_class_get_methods(mclass, &iter))) {
                if (strcmp(mono_method_get_name(method), name) != 0) { continue; }

                MonoMethodSignature *sig = mono_method_signature(method);
                if (mono_signature_get_param_count(sig) != num_args) { continue; }

                MonoType *mt = nullptr;
                gpointer iter = nullptr;
                bool match = true;
                for (int ia = 0; ia < num_args; ++ia) {
                    mt = mono_signature_get_params(sig, &iter);
                    if (strcmp(mono_type_get_name(mt), arg_typenames[ia]) != 0) {
                        match = false;
                        break;
                    }
                }
                if (match) { return method; }
            }
        }
    }
    else {
        for (mClass mc = mclass; mc; mc = mc.getParent()) {
            if (MonoMethod *ret = mono_class_get_method_from_name(mc.mclass, name, num_args)) {
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
    while ((field = mono_class_get_fields(mclass, &iter))) {
        mField mf = field;
        f(mf);
    }
}

void mClass::eachProperties(const std::function<void(mProperty&)> &f)
{
    MonoProperty *prop = nullptr;
    gpointer iter = nullptr;
    while ((prop = mono_class_get_properties(mclass, &iter))) {
        mProperty mp = prop;
        f(mp);
    }
}

void mClass::eachMethods(const std::function<void(mMethod&)> &f)
{
    MonoMethod *method = nullptr;
    gpointer iter = nullptr;
    while ((method = mono_class_get_methods(mclass, &iter))) {
        mMethod mm = method;
        f(mm);
    }
}

void mClass::eachFieldsUpwards(const std::function<void(mField&, mClass&)> &f)
{
    mClass c = mclass;
    do {
        MonoClassField *field = nullptr;
        gpointer iter = nullptr;
        while (field = mono_class_get_fields(c.mclass, &iter)) {
            mField m = field;
            f(m, c);
        }
        c = c.getParent();
    } while (c);
}

void mClass::eachPropertiesUpwards(const std::function<void(mProperty&, mClass&)> &f)
{
    mClass c = mclass;
    do {
        MonoProperty *prop = nullptr;
        gpointer iter = nullptr;
        while (prop = mono_class_get_properties(c.mclass, &iter)) {
            mProperty m = prop;
            f(m, c);
        }
        c = c.getParent();
    } while (c);
}

void mClass::eachMethodsUpwards(const std::function<void(mMethod&, mClass&)> &f)
{
    mClass c = mclass;
    do {
        MonoMethod *method = nullptr;
        gpointer iter = nullptr;
        while (method = mono_class_get_methods(c.mclass, &iter)) {
            mMethod m = method;
            f(m, c);
        }
        c = c.getParent();
    } while (c);
}


mClass mClass::getParent() const
{
    return mono_class_get_parent(mclass);
}


mClass mObject::getClass() const
{
    return mono_object_get_class(mobj);
}

MonoDomain* mObject::getDomain() const
{
    return mono_object_get_domain(mobj);
}

/*static*/ mObject mObject::New(mClass mclass)
{
    return mono_object_new(mono_domain_get(), mclass);
}


void* mObject::data()
{
    return mono_object_unbox(mobj);
}

mField mObject::findField(const char *name) const
{
    return getClass().findField(name);
}
mProperty mObject::findProperty(const char *name) const
{
    return getClass().findProperty(name);
}
mMethod mObject::findMethod(const char *name, int num_args, const char **arg_typenames) const
{
    return getClass().findMethod(name, num_args, arg_typenames);
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
    return mono_class_is_subclass_of(parent, child, false) != 0;
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

const mchar8* mString::toUTF8()
{
    return mono_string_to_utf8((MonoString*)mobj);
}

const mchar16* mString::toUTF16()
{
    return mono_string_to_utf16((MonoString*)mobj);
}


/*static*/ mArray mArray::New(mClass klass, size_t size)
{
    MonoArray *ret = mono_array_new(mono_domain_get(), klass, (mono_array_size_t)size);
    return ret;
}

size_t mArray::size() const
{
    return ((MonoArray*)mobj)->max_length;
}
void* mArray::data()
{
    return ((MonoArray*)mobj)->vector;
}


#define DefBuiltinType(Type, ClassGetter, Typename)\
    template<> mClass mTypeinfo<Type>() { static mCachedClass s_class(ClassGetter); return s_class; }\
    template<> const char* mTypename<Type>() { return Typename; }

DefBuiltinType(bool, mono_get_boolean_class, "System.Boolean");
DefBuiltinType(uint8_t, mono_get_byte_class, "System.Byte");
DefBuiltinType(int, mono_get_int32_class, "System.Int32");
DefBuiltinType(float, mono_get_single_class, "System.Single");
DefBuiltinType(mString, mono_get_string_class, "System.String");

uint32_t mPin(mObject obj)
{
    return mono_gchandle_new(obj, 1);
}

void mUnpin(uint32_t handle)
{
    mono_gchandle_free(handle);
}





static struct mCaches
{
    std::vector<mCachedImage*>    images;
    std::vector<mCachedClass*>    classes;
    std::vector<mCachedField*>    fields;
    std::vector<mCachedMethod*>   methods;
    std::vector<mCachedProperty*> properties;
} g_mCaches;

void mClearCache()
{
    for (auto o : g_mCaches.images) { o->mimage = nullptr; }
    for (auto o : g_mCaches.classes) { o->mclass = nullptr; }
    for (auto o : g_mCaches.methods) { o->mmethod = nullptr; }
    for (auto o : g_mCaches.properties) { o->mproperty = nullptr; }
    for (auto o : g_mCaches.fields) { o->mfield = nullptr; }
}

void mRebindCache()
{
    for (auto o : g_mCaches.images) { o->rebind(); }
    for (auto o : g_mCaches.classes) { o->rebind(); }
    for (auto o : g_mCaches.methods) { o->rebind(); }
    for (auto o : g_mCaches.properties) { o->rebind(); }
    for (auto o : g_mCaches.fields) { o->rebind(); }
}


mCachedImage::mCachedImage(const char *name)
    : mImage(nullptr)
    , m_name(name)
{
    rebind();
    g_mCaches.images.push_back(this);
}
void mCachedImage::clear() { mimage = nullptr; }
void mCachedImage::rebind() { mimage = findImage(m_name); }


mCachedClass::mCachedClass(mCachedImage& img, const char *ns, const char *name)
    : mClass(nullptr)
    , m_image(&img)
    , m_namespace(ns)
    , m_name(name)
{
    g_mCaches.classes.push_back(this);
}

mCachedClass::mCachedClass(Initializer init)
    : mClass(nullptr)
    , m_initializer(init)
{
}

void mCachedClass::clear() { mclass = nullptr; }
void mCachedClass::rebind() { mclass = m_initializer ? m_initializer() : m_image->findClass(m_namespace, m_name); }


mCachedField::mCachedField(mCachedClass& mclass, const char *name)
    : mField(nullptr)
    , m_class(&mclass)
    , m_name(name)
{
    rebind();
    g_mCaches.fields.push_back(this);
}
void mCachedField::clear() { mfield = nullptr; }
void mCachedField::rebind() { mfield = m_class->findField(m_name); }


mCachedMethod::mCachedMethod(mCachedClass& mclass, const char *name, int nargs, const char **arg_types)
    : mMethod(nullptr)
    , m_class(&mclass)
    , m_name(name)
    , m_num_args(nargs)
    , m_arg_typenames(arg_types)
{
    rebind();
    g_mCaches.methods.push_back(this);
}
void mCachedMethod::clear() { mmethod = nullptr; }
void mCachedMethod::rebind() { mmethod = m_class->findMethod(m_name, m_num_args, m_arg_typenames); }


mCachedProperty::mCachedProperty(mCachedClass& mclass, const char *name)
    : mProperty(nullptr)
    , m_class(&mclass)
    , m_name(name)
{
    rebind();
    g_mCaches.properties.push_back(this);
}
void mCachedProperty::clear() { mproperty = nullptr; }
void mCachedProperty::rebind() { mproperty = m_class->findProperty(m_name); }
