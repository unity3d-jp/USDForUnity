#pragma once

typedef char        mchar8;
typedef uint16_t    mchar16;

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
struct MonoMList;

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

template<class T> class mPinned;
template<class T> class mManaged;
template<class T> class mTArray;
template<class T> using mPTArray = mPinned<mTArray<T>>;
template<class T> using mMTArray = mManaged<mTArray<T>>;

template<class T> inline mClass&     mTypeof();
template<class T> inline const char* mTypename();
template<class T> inline const char* mTypenameRef();
template<class T> inline const char* mTypenameArray();


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
    bool operator==(const mDomain& other) const { return m_rep == other.m_rep; }
    bool operator!=(const mDomain& other) const { return m_rep != other.m_rep; }
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
    bool operator==(const mAssembly& other) const { return m_rep == other.m_rep; }
    bool operator!=(const mAssembly& other) const { return m_rep != other.m_rep; }
    MonoAssembly* get() { return m_rep; }
    std::string getAssemblyName() const;

private:
    MonoAssembly *m_rep;
};


class mImage
{
public:
    mImage(MonoImage *m) : m_rep(m) {}
    operator bool() const { return m_rep != nullptr; }
    bool operator==(const mImage& other) const { return m_rep == other.m_rep; }
    bool operator!=(const mImage& other) const { return m_rep != other.m_rep; }
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
    bool operator==(const mType& other) const { return m_rep == other.m_rep; }
    bool operator!=(const mType& other) const { return m_rep != other.m_rep; }
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
    bool operator==(const mField& other) const { return m_rep == other.m_rep; }
    bool operator!=(const mField& other) const { return m_rep != other.m_rep; }
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
    bool operator==(const mProperty& other) const { return m_rep == other.m_rep; }
    bool operator!=(const mProperty& other) const { return m_rep != other.m_rep; }
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
    bool operator==(const mMethod& other) const { return m_rep == other.m_rep; }
    bool operator!=(const mMethod& other) const { return m_rep != other.m_rep; }
    MonoMethod* get() const { return m_rep; }

    const char* getName() const;
    mClass getClass() const;
    mObject invoke(mObject obj, void **args=nullptr);
    mMethod inflate(mClass *params, size_t nparams, void *& mem);

protected:
    MonoMethod *m_rep;
};


class mClass
{
public:
    mClass(MonoClass *mc) : m_rep(mc) {}
    operator bool() const { return m_rep != nullptr; }
    bool operator==(const mClass& other) const { return m_rep == other.m_rep; }
    bool operator!=(const mClass& other) const { return m_rep != other.m_rep; }

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
    operator bool() const { return m_rep != nullptr; }
    bool operator==(const mObject& other) const { return m_rep == other.m_rep; }
    bool operator!=(const mObject& other) const { return m_rep != other.m_rep; }
    MonoObject* get() const { return m_rep; }

    MonoDomain* getDomain() const;
    mClass      getClass() const;
    void*       unbox();
    void*       unboxValue();
    template<class T> T& unbox() { return *(T*)unbox(); }
    template<class T> T& unboxValue() { return *(T*)unboxValue(); }
    template<class T> T& unbox() const { return *(T*)const_cast<mObject*>(this)->unbox(); }
    template<class T> T& unboxValue() const { return *(T*)const_cast<mObject*>(this)->unboxValue(); }

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
    bool operator==(const mString& other) const { return m_rep == other.m_rep; }
    bool operator!=(const mString& other) const { return m_rep != other.m_rep; }
    MonoString* get() const { return (MonoString*)m_rep; }

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
    bool operator==(const mArray& other) const { return m_rep == other.m_rep; }
    bool operator!=(const mArray& other) const { return m_rep != other.m_rep; }
    MonoArray* get() const { return (MonoArray*)m_rep; }

    size_t size() const;
    void* data();
};




// gc control

uint32_t mGCHandleAllocate(mObject obj, bool pin);
void     mGCHandleFree(uint32_t handle);
mObject  mGCHandleGetObject(uint32_t handle);


template<class T> inline void mResize(mTArray<T>& a, size_t s);
template<class T> inline void mResize(mPTArray<T>& a, size_t s);
template<class T> inline void mResize(mMTArray<T>& a, size_t s);


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

mObject mGetSystemType(mClass c);
template<class T> mObject mGetSystemType() { return mGetSystemType(mTypeof<T>()); }



// debug mechanism

#define mThisClass mTypeof<std::remove_reference<decltype(*this)>::type>()

#ifdef usdiDebug
    #define mTypeCheck(T1, T2)\
        if(T1 != T2) {\
            assert(T1 == T2);\
        }
    #define mTypeCheckThis()\
        if(!isNull() && getClass() != mThisClass) {\
            assert(getClass() == mThisClass);\
        }
#else
    #define mTypeCheck(...)
    #define mTypeCheckThis()
#endif

#include "MonoWrapperImpl.h"
