#pragma once


template<class T> struct mTypeofImpl
{
    static inline mClass& mTypeof() { return T::_getClass(); }
    static inline const char* mTypename() { return T::_getTypename(); }
    static inline const char* mTypenameRef() { return T::_getTypenameRef(); }
    static inline const char* mTypenameArray() { return T::_getTypenameArray(); }
};
template<class T> struct mTypeofImpl<mPinned<T>> : mTypeofImpl<T>
{
};
template<class T> struct mTypeofImpl<mManaged<T>> : mTypeofImpl<T>
{
};

template<class T> inline mClass&     mTypeof() { return mTypeofImpl<T>::mTypeof(); }
template<class T> inline const char* mTypename() { return mTypeofImpl<T>::mTypename(); }
template<class T> inline const char* mTypenameRef() { return mTypeofImpl<T>::mTypenameRef(); }
template<class T> inline const char* mTypenameArray() { return mTypeofImpl<T>::mTypenameArray(); }


template<class T> struct mRemovePinned { using type = T; };
template<class T> struct mRemovePinned<mPinned<T>> { using type = T; };
template<class T> using mRemovePinnedT = typename mRemovePinned<T>::type;

template<class T> struct mRemoveManaged { using type = T; };
template<class T> struct mRemoveManaged<mManaged<T>> { using type = T; };
template<class T> using mRemoveManagedT = typename mRemoveManaged<T>::type;

template<class T> struct mRemovePM { using type = typename mRemoveManaged<typename mRemovePinned<T>::type>::type; };
template<class T> using mRemovePMT = typename mRemovePM<T>::type;


// builtin types

#define DeclBuiltinType(MonoType, ValueType)\
    struct MonoType\
    {\
        mDeclTraits();\
        operator ValueType&() { return value; }\
        ValueType value = {};\
    };

struct mVoid { mDeclTraits(); };
DeclBuiltinType(mIntPtr, void*);
DeclBuiltinType(mBool, int);
DeclBuiltinType(mByte, uint8_t);
DeclBuiltinType(mInt32, int32_t);
DeclBuiltinType(mEnum, int32_t);
DeclBuiltinType(mSingle, float);
DeclBuiltinType(mDouble, double);
#undef DeclBuiltinType

mDeclImage(mscorlib);


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
    reference       operator[](size_t i) { return data()[i]; }
    const_reference operator[](size_t i) const { return data()[i]; }
    iterator        begin() { return data(); }
    iterator        end() { return data() + size(); }
    const_iterator  begin() const { return data(); }
    const_iterator  end() const { return data() + size(); }
};


template<class T>
class mPinned
{
public:
    mPinned(const mPinned& v) = delete;
    mPinned& operator=(const mPinned& v) = delete;

    mPinned() {}
    mPinned(T v) { reset(v); }
    mPinned(mPinned&& v) : m_obj(v.m_obj), m_gch(v.m_gch) { v.m_obj = nullptr; v.m_gch = 0; }
    ~mPinned() { reset(); }
    mPinned& operator=(const mPinned&& v)
    {
        reset();
        std::swap(m_obj, v.m_obj);
        std::swap(m_gch, v.m_gch);
        return *this;
    }

    operator bool() const { return (bool)m_obj; }
    T&       operator*() { return m_obj; }
    const T& operator*() const { return m_obj; }
    T*       operator->() { return &m_obj; }
    const T* operator->() const { return &m_obj; }
    T&       get() { return m_obj; }
    const T& get() const { return m_obj; }

    void reset()
    {
        if (m_gch) {
            mUnpin(m_obj);
            m_gch = 0;
            m_obj = nullptr;
        }
    }

    void reset(T obj)
    {
        reset();
        m_obj = obj;
        if (m_obj.get())
        {
            m_gch = mPin(m_obj);
        }
    }
protected:
    T m_obj;
    uint32_t m_gch = 0;
};


template<class T>
class mManaged
{
public:
    mManaged(const mManaged& v) = delete;
    mManaged& operator=(const mManaged& v) = delete;

    mManaged() {}
    mManaged(T v) { reset(v); }
    mManaged(mManaged&& v) : m_ml(v.m_ml) { v.m_ml = nullptr; }
    ~mManaged() { reset(); }
    mManaged& operator=(const mManaged&& v)
    {
        reset();
        std::swap(m_ml, v.m_ml);
        return *this;
    }

    operator bool() const { return m_ml && (bool)get(); }
    T&       operator*() { return get(); }
    const T& operator*() const { return get(); }
    T*       operator->() { return &get(); }
    const T* operator->() const { return &get(); }
    T&       get() { m_obj = mGetObject(m_ml); return m_obj; }
    const T& get() const { m_obj = mGetObject(m_ml); return m_obj; }

    void reset()
    {
        if (m_ml) {
            mUnmanage(m_ml);
            m_ml = nullptr;
        }
    }

    void reset(T obj)
    {
        reset();
        if (v.get()) {
            m_ml = mManage(v);
        }
    }

protected:
    mutable T m_obj;
    mMList m_ml;
};


template<class T>
inline void mResize(mTArray<T>& a, size_t s)
{
    if (a.size() == s) { return; }
    a = mTArray<T>::New(s);
}

template<class T>
inline void mResize(mPTArray<T>& a, size_t s)
{
    if (a->size() == s) { return; }
    a.reset(mTArray<T>::New(s));
}

template<class T>
inline void mResize(mMTArray<T>& a, size_t s)
{
    if (a->size() == s) { return; }
    a.reset(mMTArray<T>::New(s));
}


