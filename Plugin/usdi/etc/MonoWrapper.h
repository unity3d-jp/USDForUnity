#pragma once

struct MonoDomain;
struct MonoAssembly;
struct MonoImage;
struct MonoType;
struct MonoClassField;
struct MonoProperty;
struct MonoMethod;
struct MonoClass;
struct MonoObject;
struct MonoString;
struct MonoArray;

class mDomain;
class mImage;
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

template<class T> mClass&     mTypeof() { return T::_getClass(); }
template<class T> const char* mTypename() { return T::_getTypename(); }
template<class T> const char* mTypenameRef() { return T::_getTypenameRef(); }
template<class T> const char* mTypenameArray() { return T::_getTypenameArray(); }

#define mDeclImage(Name) mImage& mGet##Name##Image();
#define mDefImage(Name, AssemblyName) mImage& mGet##Name##Image() { static mImage& s_image=mCreateImageCache(AssemblyName); return s_image; }
#define mGetImage(Name) mGet##Name##Image()

#define mDeclTraits()\
    static mClass& _getClass();\
    static const char* _getTypename();\
    static const char* _getTypenameRef();\
    static const char* _getTypenameArray();

#define mDefTraits(Img, Namespace, MonoTypename, Type)\
    mClass& Type::_getClass()\
    {\
        static mClass& s_class=mCreateClassCache(mGetImage(Img), Namespace, MonoTypename);\
        return s_class;\
    }\
    const char* Type::_getTypename() { return #Namespace "." MonoTypename; }\
    const char* Type::_getTypenameRef() { return #Namespace "." MonoTypename "&"; }\
    const char* Type::_getTypenameArray() { return #Namespace "." MonoTypename "[]"; }


class mDomain
{
public:
    mDomain(MonoDomain *m) : m_rep(m) {}
    operator bool() const { return m_rep != nullptr; }
    bool operator==(mDomain other) const { return m_rep == other.m_rep; }
    bool operator!=(mDomain other) const { return m_rep != other.m_rep; }
    MonoDomain* get() { return m_rep; }

    static mImage findImage(const char *name); // name: not includes extensions. ex: "UnityEngine"

protected:
    MonoDomain *m_rep;
};

class mAssembly
{
public:
    mAssembly(MonoAssembly *m) : m_rep(m) {}
    operator bool() const { return m_rep != nullptr; }
    bool operator==(mAssembly other) const { return m_rep == other.m_rep; }
    bool operator!=(mAssembly other) const { return m_rep != other.m_rep; }
    MonoAssembly* get() { return m_rep; }
    const char* stringifyAssemblyName() const; // !!! caller must freeAssemblyName() returned string !!!
    void freeAssemblyName( const char *aname);

private:
    MonoAssembly *m_rep;
};

class mImage
{
public:
    mImage(MonoImage *m) : m_rep(m) {}
    operator bool() const { return m_rep != nullptr; }
    bool operator==(mImage other) const { return m_rep == other.m_rep; }
    bool operator!=(mImage other) const { return m_rep != other.m_rep; }
    MonoImage* get() { return m_rep; }

    mAssembly   getAssembly();
    mClass      findClass(const char *namespace_, const char *class_name);

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
    const char* getNamespace() const;
    mImage    getImage() const;
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
    mDeclTraits();
    static mObject New(mClass mclass);
    template<class T> static T New() { return T(New(mTypeof<T>()).get()); }

    mObject(MonoObject *o = nullptr) : m_rep(o) {}
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
    static mObject sinvoke(mMethod method);
    static mObject sinvoke(mMethod method, void *a0);
    static mObject sinvoke(mMethod method, void *a0, void *a1);
    static mObject sinvoke(mMethod method, void *a0, void *a1, void *a2);

protected:
    MonoObject *m_rep;
};


class mString : public mObject
{
public:
    mDeclTraits();
    static mString New(const mchar8 *str, int len = -1);
    static mString New(const mchar16 *str, int len = -1);

    mString(MonoObject *o = nullptr) : mObject(o) {}
    mString(MonoString *o) : mObject((MonoObject*)o) {}
    operator bool() const { return m_rep != nullptr; }
    MonoString* get() { return (MonoString*)m_rep; }

    size_t size() const;
    const mchar8*    toUTF8();
    const mchar16*   toUTF16();
};


class mArray : public mObject
{
public:
    static mArray New(mClass klass, size_t size);

    mArray(MonoArray *o = nullptr) : mObject((MonoObject*)o) {}
    operator bool() const { return m_rep != nullptr; }
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
};

// builtin types

#define mDeclBuiltinType(MonoType, ValueType)\
    struct MonoType\
    {\
        mDeclTraits();\
        operator ValueType&() { return value; }\
        ValueType value = {};\
    };\

struct mVoid { mDeclTraits(); };
mDeclBuiltinType(mIntPtr, void*);
mDeclBuiltinType(mBool, int);
mDeclBuiltinType(mByte, uint8_t);
mDeclBuiltinType(mInt32, int32_t);
mDeclBuiltinType(mEnum, int32_t);
mDeclBuiltinType(mSingle, float);
mDeclBuiltinType(mDouble, double);
#undef mDeclBuiltinType


// gc control

uint32_t mPin(mObject obj);
void     mUnpin(uint32_t handle);

template<class T>
class mPinned
{
typedef T super;
public:
    mPinned() {}
    mPinned(T v) { reset(v); }
    mPinned(mPinned&& v) : m_obj(v.m_obj) , m_gch(v.m_gch) { v.m_obj=nullptr; v.m_gch=0; }
    ~mPinned() { reset(); }

    mPinned(const mPinned& v) = delete;
    mPinned& operator=(const mPinned& v) = delete;
    mPinned& operator=(const mPinned&& v)
    {
        reset();
        std::swap(m_obj, v.m_obj);
        std::swap(m_gch, v.m_gch);
        return *this;
    }

    operator bool() const       { return (bool)m_obj; }
    T&       operator*()        { return m_obj; }
    const T& operator*() const  { return m_obj; }
    T*       operator->()       { return &m_obj; }
    const T* operator->() const { return &m_obj; }
    T&       get()              { return m_obj; }
    const T& get() const        { return m_obj; }

    void reset()
    {
        unpin();
        m_obj = nullptr;
    }

    void reset(T obj)
    {
        unpin();
        m_obj = obj;
        pin();
    }

protected:
    void pin()
    {
        if (m_obj && !m_gch)
        {
            m_gch = mPin(m_obj);
        }
    }

    void unpin()
    {
        if (m_gch) {
            mUnpin(m_obj);
            m_gch = 0;
        }
    }

protected:
    T m_obj;
    uint32_t m_gch = 0;
};

template<class T> using mPTArray = mPinned<mTArray<T>>;

template<class T>
void mResize(mTArray<T>& a, size_t s)
{
    if (a.size() == s) { return; }
    a = mTArray<T>::New(s);
}

template<class T>
void mResize(mPTArray<T>& a, size_t s)
{
    if (a->size() == s) { return; }
    a.reset(mTArray<T>::New(s));
}

mString mToMString(const char *s);
std::string mToCString(mObject v);

bool mIsSubclassOf(mClass parent, mClass child);
void mAddMethod(const char *name, void *addr);

mDomain& mGetDomain();
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

void mAttachThread();
void mDetachThread();
void mDetachAllThreads();



// debug mechanism

#define mThisClass mTypeof<std::remove_reference<decltype(*this)>::type>()

#ifdef usdiDebug
    #define mTypeCheck(T1, T2)\
        if((T1)!=(T2)) {\
            auto *typename1 = (T1).getName();\
            auto *typename2 = (T2).getName();\
            assert((T1)!=(T2));\
        }
    #define mTypeCheckThis()\
        if(!isNull() && getClass() != mThisClass) {\
            auto *type_name = getClass().getName();\
            assert(getClass() == mThisClass);\
        }
#else
    #define mTypeCheck(...)
    #define mTypeCheckThis()
#endif
