namespace usdi {

template<class T>
T* Context::createSchema(Schema *parent, const char *name)
{
    T *ret = new T(this, parent, name);
    addSchema(ret);
    if (parent) { parent->addChild(ret); }
    return ret;
}

} // namespace usdi
