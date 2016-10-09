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

template<class T> const char*   mTypename();
template<class T> const char*   mTypenameRef();
template<class T> const char*   mTypenameArray();
template<class T> mClass        mTypeinfo();


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
    const char* getName() const;

    mObject invoke(mObject obj, void **args=nullptr);
    mMethod instantiate(mClass *params, int num_params, void *& allocated_space);

public:
    MonoMethod *mmethod;
};


class mClass
{
public:
    mClass(MonoClass *mc) : mclass(mc) {}
    operator MonoClass*() const { return mclass; }
    operator bool() const { return mclass != nullptr; }
    const char* getName() const;
    mType     getType() const;
    mClass    getParent() const;

    mField    findField(const char *name) const;
    mProperty findProperty(const char *name) const;
    mMethod   findMethod(const char *name, int num_args=-1, const char **arg_typenames=nullptr) const; // num_args: -1=don't care

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
    T as() const { return mIsSubclassOf(*this, mTypeinfo<T>()) ? T(*this) : nullptr; }

public:
    MonoClass *mclass;
};


class mObject
{
public:
    static mObject New(mClass mclass);

    mObject(MonoObject *o) : mobj(o) {}
    operator MonoObject*() const { return mobj; }
    operator bool() const { return mobj != nullptr; }

    MonoDomain* getDomain() const;
    mClass    getClass() const;
    void*       data();
    template<class T> T& getValue(int offset=0) { return *(T*)((size_t)data()+offset); }

    // redirect to mClass
    mField    findField(const char *name) const;
    mProperty findProperty(const char *name) const;
    mMethod   findMethod(const char *name, int num_args = -1, const char **arg_typenames = nullptr) const;

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

    mArray(MonoObject *o) : mObject(o) {}
    mArray(MonoArray *o) : mObject((MonoObject*)o) {}

    size_t size() const;
    void* data();
};

template<class T>
class mTArray
{
public:
    typedef T           value_type;
    typedef T&          reference;
    typedef const T&    const_reference;
    typedef T*          pointer;
    typedef const T*    const_pointer;
    typedef T*          iterator;
    typedef const T*    const_iterator;

    mTArray(mObject cs_array) : m_array(cs_array), m_size(m_array.size()), m_data((pointer)m_array.data()) {}
    mTArray(mArray cs_array) : m_array(cs_array), m_size(m_array.size()), m_data((pointer)m_array.data()) {}
    size_t          size() const { return m_size; }
    pointer         data() { return m_data; }
    reference       operator[](size_t i)        { return m_data[i]; }
    const_reference operator[](size_t i) const  { return m_data[i]; }
    iterator        begin()                     { return m_data; }
    iterator        end()                       { return m_data + m_size; }
    const_iterator  begin() const               { return m_data; }
    const_iterator  end() const                 { return m_data + m_size; }
    operator bool() const { return m_array; }
    operator mArray() const { return m_array; }

private:
    mArray m_array;
    size_t m_size;
    pointer m_data;
};


template<class T> const char* mTypename()       { return T::getTypename(); }
template<class T> const char* mTypenameRef()    { return T::getTypenameRef(); }
template<class T> const char* mTypenameArray()  { return T::getTypenameArray(); }
template<class T> mClass      mTypeinfo()       { return T::getClass(); }

template<> mClass       mTypeinfo<bool>();
template<> const char*  mTypename<bool>();
template<> mClass       mTypeinfo<uint8_t>();
template<> const char*  mTypename<uint8_t>();
template<> mClass       mTypeinfo<int>();
template<> const char*  mTypename<int>();
template<> mClass       mTypeinfo<float>();
template<> const char*  mTypename<float>();
template<> mClass       mTypeinfo<mString>();
template<> const char*  mTypename<mString>();

bool     mIsSubclassOf(mClass parent, mClass child);
void     mAddMethod(const char *name, void *addr);
uint32_t mPin(mObject obj);
void     mUnpin(uint32_t handle);

void mClearCache();
void mRebindCache();




class mCachedImage : public mImage
{
public:
    mCachedImage(const char *name);
    void clear();
    void rebind();

private:
    const char *m_name;
};

class mCachedClass : public mClass
{
public:
    typedef MonoClass* (*Initializer)();

    mCachedClass(mCachedImage& img, const char *ns, const char *name);
    mCachedClass(Initializer init);
    void clear();
    void rebind();

private:
    mCachedImage *m_image = nullptr;
    const char *m_name = nullptr;
    const char *m_namespace = nullptr;
    Initializer m_initializer = nullptr;
};

class mCachedField : public mField
{
public:
    mCachedField(mCachedClass& mclass, const char *name);
    void clear();
    void rebind();

private:
    mCachedClass *m_class;
    const char *m_name;
};

class mCachedMethod : public mMethod
{
public:
    mCachedMethod(mCachedClass& mclass, const char *name, int nargs=-1, const char **arg_types=nullptr);
    void clear();
    void rebind();

private:
    mCachedClass *m_class;
    const char *m_name;
    int m_num_args;
    const char **m_arg_typenames;
};

class mCachedProperty : public mProperty
{
public:
    mCachedProperty(mCachedClass& mclass, const char *name);
    void clear();
    void rebind();

private:
    mCachedClass *m_class;
    const char *m_name;
};


#define mDeclImage(Name) mImage mGet##Name##Image();
#define mImplImage(Name) mImage mGet##Name##Image() { static mCachedImage s_image(Name); return s_image; }

#define mDeclTraits()\
    static mClass getClass();\
    static const char* getTypename();\
    static const char* getTypenameRef();\
    static const char* getTypenameArray();


#define mImplTraits(Img, Namespace, Type)\
    mClass Type::getClass()\
    {\
        static mCachedClass s_class(mGet##Img##Image(), Namespace, Type);\
        return s_class;\
    }\
    const char* Type::getTypename() { return #Namespace "." #Type; }\
    const char* Type::getTypenameRef() { return #Namespace "." #Type "&"; }\
    const char* Type::getTypenameArray() { return #Namespace "." #Type "[]"; }

