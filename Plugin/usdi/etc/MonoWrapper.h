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

typedef std::initializer_list<const char*> mTypenames;

template<class T> mClass&       mTypeof();
template<class T> const char*   mTypename();
template<class T> const char*   mTypenameRef();
template<class T> const char*   mTypenameArray();


class mImage
{
public:
    static mImage findImage(const char *name); // name: not includes extensions. ex: "UnityEngine"

    mImage(MonoImage *m) : mimage(m) {}
    operator MonoImage*() const { return mimage; }
    operator void*() const { return mimage; }
    operator bool() const { return mimage != nullptr; }

    mClass findClass(const char *namespace_, const char *class_name);

public:
    MonoImage *mimage;
};


class mType
{
public:
    mType(MonoType *m) : mtype(m) {}
    operator MonoType*() const { return mtype; }
    operator bool() const { return mtype != nullptr; }
    bool operator==(mType other) const { return mtype == other.mtype; }
    bool operator!=(mType other) const { return mtype != other.mtype; }
    const char* getName() const;
    mClass getClass() const;

public:
    MonoType *mtype;
};


class mField
{
public:
    mField(MonoClassField *mc) : mfield(mc) {}
    operator MonoClassField*() const { return mfield; }
    operator bool() const { return mfield != nullptr; }
    const char* getName() const;

    mType getType() const;
    int getOffset() const;
    template<class T> void getValue(mObject obj, T &o) const { getValueImpl(obj, &o); }
    template<class T> void setValue(mObject obj, const T &o) { setValueImpl(obj, &o); }
    template<class T> void setValueRef(mObject obj, T *o) { setValueImpl(obj, o); }

    void getValueImpl(mObject obj, void *p) const;
    void setValueImpl(mObject obj, const void *p);

public:
    MonoClassField *mfield;
};


class mProperty
{
public:
    mProperty(MonoProperty *mc) : mproperty(mc) {}
    operator MonoProperty*() const { return mproperty; }
    operator bool() const { return mproperty != nullptr; }
    const char* getName() const;

    mMethod getGetter() const;
    mMethod getSetter() const;

public:
    MonoProperty *mproperty;
};


class mMethod
{
public:
    mMethod(MonoMethod *mm) : mmethod(mm) {}
    operator MonoMethod*() const { return mmethod; }
    operator bool() const { return mmethod != nullptr; }
    bool operator==(mMethod other) const { return mmethod == other.mmethod; }
    bool operator!=(mMethod other) const { return mmethod != other.mmethod; }
    const char* getName() const;

    mObject invoke(mObject obj, void **args=nullptr);
    mMethod instantiate(mClass *params, int num_params, void *& mem);

public:
    MonoMethod *mmethod;
};


class mClass
{
public:
    mClass(MonoClass *mc) : mclass(mc) {}
    operator MonoClass*() const { return mclass; }
    operator bool() const { return mclass != nullptr; }
    bool operator==(mClass other) const { return mclass == other.mclass; }
    bool operator!=(mClass other) const { return mclass != other.mclass; }

    const char* getName() const;
    mType     getType() const;
    mClass    getParent() const;

    mField    findField(const char *name) const;
    mProperty findProperty(const char *name) const;
    mMethod   findMethod(const char *name, int num_args = -1) const; // num_args: -1=don't care
    mMethod   findMethod(const char *name, mTypenames typenames) const;

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

public:
    MonoClass *mclass;
};


class mObject
{
public:
    static mObject New(mClass mclass);
    template<class T> static T New() { return T(New(mTypeof<T>()).get()); }

    mObject(MonoObject *o) : mobj(o) {}
    operator bool() const { return !isNull(); }
    bool operator==(mObject other) const { return mobj == other.mobj; }
    bool operator!=(mObject other) const { return mobj != other.mobj; }

    bool        isNull() const;
    MonoObject* get() const;
    MonoDomain* getDomain() const;
    mClass      getClass() const;
    void*       unbox();
    void*       unboxValue();
    template<class T> T& unbox() { return *(T*)unbox(); }
    template<class T> T& unboxValue() { return *(T*)unboxValue(); }

    // redirect to mClass
    mField    findField(const char *name) const;
    mProperty findProperty(const char *name) const;
    mMethod   findMethod(const char *name, int num_args = -1) const;
    mMethod   findMethod(const char *name, mTypenames typenames) const;

    mObject invoke(mMethod method);
    mObject invoke(mMethod method, void *a0);
    mObject invoke(mMethod method, void *a0, void *a1);
    mObject invoke(mMethod method, void *a0, void *a1, void *a2);

public:
    MonoObject *mobj;
};


class mString : public mObject
{
public:
    static mString New(const mchar8 *str, int len = -1);
    static mString New(const mchar16 *str, int len = -1);

    mString(MonoObject *o) : mObject(o) {}
    mString(MonoString *o) : mObject((MonoObject*)o) {}

    const mchar8*    toUTF8();
    const mchar16*   toUTF16();
};

class mArray : public mObject
{
public:
    static mArray New(mClass klass, size_t size);

    mArray(MonoArray *o) : mObject((MonoObject*)o) {}

    MonoArray* get() { return (MonoArray*)mobj; }

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


template<class T> mClass&     mTypeof()         { return T::_getClass(); }
template<class T> const char* mTypename()       { return T::_getTypename(); }
template<class T> const char* mTypenameRef()    { return T::_getTypenameRef(); }
template<class T> const char* mTypenameArray()  { return T::_getTypenameArray(); }

template<> mClass&      mTypeof<void*>();
template<> const char*  mTypename<void*>();
template<> mClass&      mTypeof<bool>();
template<> const char*  mTypename<bool>();
template<> mClass&      mTypeof<uint8_t>();
template<> const char*  mTypename<uint8_t>();
template<> mClass&      mTypeof<int>();
template<> const char*  mTypename<int>();
template<> mClass&      mTypeof<float>();
template<> const char*  mTypename<float>();
template<> mClass&      mTypeof<mString>();
template<> const char*  mTypename<mString>();

bool     mIsSubclassOf(mClass parent, mClass child);
void     mAddMethod(const char *name, void *addr);
uint32_t mPin(mObject obj);
void     mUnpin(uint32_t handle);

void mClearCache();
void mRebindCache();



class mICache
{
public:
    virtual ~mICache() {}
    virtual void clear() = 0;
    virtual void rebind() = 0;
};

class mCachedImage : public mImage, public mICache
{
public:
    mCachedImage(const char *name);
    void clear();
    void rebind();

private:
    const char *m_name;
};

class mCachedClass : public mClass, public mICache
{
public:
    typedef MonoClass* (*Initializer)();

    mCachedClass(mImage& img, const char *ns, const char *name);
    mCachedClass(Initializer init);
    void clear() override;
    void rebind() override;

private:
    mImage *m_image = nullptr;
    const char *m_name = nullptr;
    const char *m_namespace = nullptr;
    Initializer m_initializer = nullptr;
};

class mCachedField : public mField, public mICache
{
public:
    mCachedField(mClass& mclass, const char *name);
    void clear() override;
    void rebind() override;

private:
    mClass *m_class;
    const char *m_name;
};

class mCachedMethod : public mMethod, public mICache
{
public:
    mCachedMethod(mClass& mclass, const char *name, int nargs = -1);
    mCachedMethod(mClass& mclass, const char *name, mTypenames arg_types);
    void clear() override;
    void rebind() override;

private:
    mClass *m_class;
    const char *m_name;
    int m_num_args = -1;
    mTypenames m_argtypes;
};

class mCachedIMethod : public mMethod, public mICache
{
public:
    mCachedIMethod(mMethod& generics, mClass& param);
    template<class T> mCachedIMethod(mMethod& generics) { mCachedIMethod(generics, mTypeof<T>()); }
    void clear() override;
    void rebind() override;

private:
    mMethod *m_generics;
    mClass *m_param;
    void *m_mem = nullptr;
};

class mCachedProperty : public mProperty, public mICache
{
public:
    mCachedProperty(mClass& mclass, const char *name);
    void clear() override;
    void rebind() override;

private:
    mClass *m_class;
    const char *m_name;
};


#define mDeclImage(Name) mImage& mGet##Name##Image();
#define mDefImage(Name, AssemblyName) mImage& mGet##Name##Image() { static mCachedImage s_image(AssemblyName); return s_image; }

#define mDeclTraits()\
    static mClass& _getClass();\
    static const char* _getTypename();\
    static const char* _getTypenameRef();\
    static const char* _getTypenameArray();


#define mDefTraits(Img, Namespace, MonoTypename, Type)\
    mClass& Type::_getClass()\
    {\
        static mCachedClass s_class(mGet##Img##Image(), Namespace, MonoTypename);\
        return s_class;\
    }\
    const char* Type::_getTypename() { return #Namespace "." MonoTypename; }\
    const char* Type::_getTypenameRef() { return #Namespace "." MonoTypename "&"; }\
    const char* Type::_getTypenameArray() { return #Namespace "." MonoTypename "[]"; }

