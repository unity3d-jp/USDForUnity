#include "pch.h"
#ifdef usdiEnableMonoBinding
#include "Mono.h"
#include "MonoWrapper.h"

// cache manager

class mICache
{
public:
    virtual ~mICache() {}
    virtual void clear() = 0;
    virtual void rebind() = 0;
};

class mDomainCache : public mDomain, public mICache
{
public:
    mDomainCache();
    void clear();
    void rebind();
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
    mMethodCache(mClass& mclass, const char *name, const std::vector<const char*> arg_types);
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
    mIMethodCache(mMethod& generics, const std::vector<mClass*>& param);
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

mDomainCache::mDomainCache() : mDomain(nullptr) { mRegisterCache(this); }
void mDomainCache::clear() { m_rep = nullptr; }
void mDomainCache::rebind() { m_rep = _mono_domain_get(); }

mDomain& mGetDomain()
{
    static mDomainCache s_domain;
    return s_domain;
}

mImageCache::mImageCache(const char *name)
    : mImage(nullptr)
    , m_name(name)
{
    mRegisterCache(this);
}
void mImageCache::clear() { m_rep = nullptr; }
void mImageCache::rebind() { m_rep = mGetDomain().findImage(m_name).get(); }


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
    mRegisterCache(this);
}

void mClassCache::clear() { m_rep = nullptr; }
void mClassCache::rebind() {
    if (m_initializer) {
        m_rep = m_initializer();
    }
    else {
        m_rep = m_image->findClass(m_namespace, m_name).get();
    }
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
mMethodCache::mMethodCache(mClass& mclass, const char *name, const std::vector<const char*> typenames)
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

mIMethodCache::mIMethodCache(mMethod& generics, const std::vector<mClass*>& param)
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
    m_rep = m_generics->inflate(params.data(), params.size(), m_mem).get();
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
mMethod& mCreateMethodCache(mClass& mclass, const char *name, const std::vector<const char*> typenames)
{
    return *new mMethodCache(mclass, name, typenames);
}
mMethod& mCreateMethodCache(mMethod& generics, const std::vector<mClass*> params)
{
    return *new mIMethodCache(generics, params);
}
mProperty& mCreatePropertyCache(mClass& mclass, const char *name)
{
    return *new mPropertyCache(mclass, name);
}
#endif // usdiEnableMonoBinding
