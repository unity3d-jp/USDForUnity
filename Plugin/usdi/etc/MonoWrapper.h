#pragma once

struct MonoDomain;
struct MonoImage;
struct MonoType;
struct MonoClassField;
struct MonoProperty;
struct MonoMethod;
struct MonoClass;
struct MonoObject;
struct MonoString;
struct MonoArray;

class mType;
class mClass;
class mMethod;
class mField;
class mProperty;
class mObject;
class mArray;
class mString;

typedef char        mchar8;
typedef uint16_t    mchar16;

template<class T> mClass&       mTypeof();
template<class T> const char*   mTypename();
template<class T> const char*   mTypenameRef();
template<class T> const char*   mTypenameArray();


class mImage
{
public:
    static mImage findImage(const char *name); // name: not includes extensions. ex: "UnityEngine"

    mImage(MonoImage *m) : m_rep(m) {}
    operator bool() const { return m_rep != nullptr; }
    bool operator==(mImage other) const { return m_rep == other.m_rep; }
    bool operator!=(mImage other) const { return m_rep != other.m_rep; }
    MonoImage* get() { return m_rep; }

    mClass findClass(const char *namespace_, const char *class_name);

protected:
    MonoImage *m_rep;
};


class mType
{
public:
    mType(MonoType *m) : m_rep(m) {}
    operator bool() const { return m_rep != nullptr; }
    bool operator==(mType other) const { return m_rep == other.m_rep; }
    bool operator!=(mType other) const { return m_rep != other.m_rep; }
    MonoType* get() const { return m_rep; }

    const char* getName() const;
    mClass getClass() const;

protected:
    MonoType *m_rep;
};


class mField
{
public:
    mField(MonoClassField *mc) : m_rep(mc) {}
    operator bool() const { return m_rep != nullptr; }
    bool operator==(mField other) const { return m_rep == other.m_rep; }
    bool operator!=(mField other) const { return m_rep != other.m_rep; }
    MonoClassField* get() const { return m_rep; }

    const char* getName() const;
    mType getType() const;
    int getOffset() const;
    template<class T> void getValue(mObject obj, T &o) const { getValueImpl(obj, &o); }
    template<class T> void setValue(mObject obj, const T &o) { setValueImpl(obj, &o); }
    template<class T> void setValueRef(mObject obj, T *o) { setValueImpl(obj, o); }

    void getValueImpl(mObject obj, void *p) const;
    void setValueImpl(mObject obj, const void *p);

protected:
    MonoClassField *m_rep;
};


class mProperty
{
public:
    mProperty(MonoProperty *mc) : m_rep(mc) {}
    operator bool() const { return m_rep != nullptr; }
    bool operator==(mProperty other) const { return m_rep == other.m_rep; }
    bool operator!=(mProperty other) const { return m_rep != other.m_rep; }
    MonoProperty* get() const { return m_rep; }

    const char* getName() const;
    mMethod getGetter() const;
    mMethod getSetter() const;

protected:
    MonoProperty *m_rep;
};


class mMethod
{
public:
    mMethod(MonoMethod *mm) : m_rep(mm) {}
    operator bool() const { return m_rep != nullptr; }
    bool operator==(mMethod other) const { return m_rep == other.m_rep; }
    bool operator!=(mMethod other) const { return m_rep != other.m_rep; }
    MonoMethod* get() const { return m_rep; }

    const char* getName() const;
    mObject invoke(mObject obj, void **args=nullptr);
    mMethod instantiate(mClass *params, size_t nparams, void *& mem);

protected:
    MonoMethod *m_rep;
};


class mClass
{
public:
    mClass(MonoClass *mc) : m_rep(mc) {}
    operator bool() const { return m_rep != nullptr; }
    bool operator==(mClass other) const { return m_rep == other.m_rep; }
    bool operator!=(mClass other) const { return m_rep != other.m_rep; }

    MonoClass* get() const { return m_rep; }
    const char* getName() const;
    mType     getType() const;
    mClass    getParent() const;

    mField    findField(const char *name) const;
    mProperty findProperty(const char *name) const;
    mMethod   findMethod(const char *name, int num_args = -1, const char **typenames=nullptr) const; // num_args: -1=don't care

    // enumerate members (not include parent class members)
    void eachFields(const std::function<void(mField&)> &f);
    void eachProperties(const std::function<void(mProperty&)> &f);
    void eachMethods(const std::function<void(mMethod&)> &f);
    // include parent classes
    void eachFieldsUpwards(const std::function<void(mField&, mClass&)> &f);
    void eachPropertiesUpwards(const std::function<void(mProperty&, mClass&)> &f);
    void eachMethodsUpwards(const std::function<void(mMethod&, mClass&)> &f);

    //mClass insantiate(mClass *template_params);

    template<class T>
    T as() const { return mIsSubclassOf(*this, mTypeof<T>()) ? T(*this) : nullptr; }

protected:
    MonoClass *m_rep;
};


class mObject
{
public:
    static mObject New(mClass mclass);
    template<class T> static T New() { return T(New(mTypeof<T>()).get()); }

    mObject(MonoObject *o) : m_rep(o) {}
    operator bool() const { return !isNull(); }
    bool operator==(mObject other) const { return m_rep == other.m_rep; }
    bool operator!=(mObject other) const { return m_rep != other.m_rep; }
    MonoObject* get() const { return m_rep; }

    bool        isNull() const;
    MonoDomain* getDomain() const;
    mClass      getClass() const;
    void*       unbox();
    void*       unboxValue();
    template<class T> T& unbox() { return *(T*)unbox(); }
    template<class T> T& unboxValue() { return *(T*)unboxValue(); }

    // redirect to mClass
    mField    findField(const char *name) const;
    mProperty findProperty(const char *name) const;
    mMethod   findMethod(const char *name, int num_args = -1, const char **typenames = nullptr) const;

    mObject invoke(mMethod method);
    mObject invoke(mMethod method, void *a0);
    mObject invoke(mMethod method, void *a0, void *a1);
    mObject invoke(mMethod method, void *a0, void *a1, void *a2);

protected:
    MonoObject *m_rep;
};


class mString : public mObject
{
public:
    static mString New(const mchar8 *str, int len = -1);
    static mString New(const mchar16 *str, int len = -1);

    mString(MonoObject *o) : mObject(o) {}
    mString(MonoString *o) : mObject((MonoObject*)o) {}
    MonoString* get() { return (MonoString*)m_rep; }

    size_t size() const;
    const mchar8*    toUTF8();
    const mchar16*   toUTF16();
};

class mArray : public mObject
{
public:
    static mArray New(mClass klass, size_t size);

    mArray(MonoArray *o) : mObject((MonoObject*)o) {}
    MonoArray* get() { return (MonoArray*)m_rep; }

    size_t size() const;
    void* data();
};

template<class T>
class mTArray : public mArray
{
typedef mArray super;
public:
    typedef T           value_type;
    typedef T&          reference;
    typedef const T&    const_reference;
    typedef T*          pointer;
    typedef const T*    const_pointer;
    typedef T*          iterator;
    typedef const T*    const_iterator;

    static mTArray New(size_t size) { return mTArray<T>(super::New(mTypeof<T>(), size).get()); }

    mTArray(MonoArray *o = nullptr) : super(o) {}
    pointer         data() { return (pointer)super::data(); }
    reference       operator[](size_t i)        { return data()[i]; }
    const_reference operator[](size_t i) const  { return data()[i]; }
    iterator        begin()                     { return data(); }
    iterator        end()                       { return data() + size(); }
    const_iterator  begin() const               { return data(); }
    const_iterator  end() const                 { return data() + size(); }

    void resize(size_t s)
    {
        if (size() == s) { return; }
        *this = New(s);
    }

    void clear()
    {
    }
};


template<class T> mClass&     mTypeof() { return T::_getClass(); }
template<class T> const char* mTypename() { return T::_getTypename(); }
template<class T> const char* mTypenameRef() { return T::_getTypenameRef(); }
template<class T> const char* mTypenameArray() { return T::_getTypenameArray(); }

#define DeclBuiltinType(T) template<> mClass& mTypeof<T>(); template<> const char* mTypename<T>();
DeclBuiltinType(void*);
DeclBuiltinType(bool);
DeclBuiltinType(uint8_t);
DeclBuiltinType(int);
DeclBuiltinType(float);
DeclBuiltinType(mString);
#undef DeclBuiltinType


bool     mIsSubclassOf(mClass parent, mClass child);
void     mAddMethod(const char *name, void *addr);
uint32_t mPin(mObject obj);
void     mUnpin(uint32_t handle);

mImage& mCreateImageCache(const char *name);
mClass& mCreateClassCache(mImage& img, const char *ns, const char *name);
mClass& mCreateClassCache(MonoClass* (*initializer)());
mField& mCreateFieldCache(mClass& mclass, const char *name);
mMethod& mCreateMethodCache(mClass& mclass, const char *name, int nargs = -1);
mMethod& mCreateMethodCache(mClass& mclass, const char *name, std::vector<const char*> typenames);
mMethod& mCreateMethodCache(mMethod& generics, std::vector<mClass*> params);
mProperty& mCreatePropertyCache(mClass& mclass, const char *name);
void mClearCache();
void mRebindCache();



#define mDeclImage(Name) mImage& mGet##Name##Image();
#define mDefImage(Name, AssemblyName) mImage& mGet##Name##Image() { static mImage& s_image=mCreateImageCache(AssemblyName); return s_image; }

#define mDeclTraits()\
    static mClass& _getClass();\
    static const char* _getTypename();\
    static const char* _getTypenameRef();\
    static const char* _getTypenameArray();


#define mDefTraits(Img, Namespace, MonoTypename, Type)\
    mClass& Type::_getClass()\
    {\
        static mClass& s_class=mCreateClassCache(mGet##Img##Image(), Namespace, MonoTypename);\
        return s_class;\
    }\
    const char* Type::_getTypename() { return #Namespace "." MonoTypename; }\
    const char* Type::_getTypenameRef() { return #Namespace "." MonoTypename "&"; }\
    const char* Type::_getTypenameArray() { return #Namespace "." MonoTypename "[]"; }

