#pragma once

namespace usdi {

template<class T> T* RawPtr(T *v) { return v; }
template<class T> T* RawPtr(const std::unique_ptr<T>& v) { return v.get(); }

template<class SchemaArray>
inline Schema* FindSchema(SchemaArray& schemas, const char *path)
{
    if (path[0] == '/') {
        for (auto& n : schemas) {
            if (strcmp(n->getPath(), path) == 0) {
                return RawPtr(n);
            }
        }
    }
    else {
        // search node that has specified name
        for (auto& n : schemas) {
            if (strcmp(n->getName(), path) == 0) {
                return RawPtr(n);
            }
        }
    }
    return nullptr;
}


typedef RawVector<char> TempBuffer;
TempBuffer& GetTemporaryBuffer();


template<typename Body>
class lambda_task : public tbb::task
{
private:
    Body m_body;
    tbb::task* execute() override
    {
        m_body();
        return nullptr;
    }
public:
    lambda_task(const Body& body) : m_body(body) {}
};

template<typename Body>
inline tbb::task* launch(const Body& body)
{
    auto *ret = new(tbb::task::allocate_root()) lambda_task<Body>(body);
    tbb::task::enqueue(*ret);
    return ret;
}

} // namespace usdi
