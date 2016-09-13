#pragma once

namespace usdi {

template<class ResultType>
class AsyncHandle
{
public:
    AsyncHandle(hasync h=0) : m_hasync(h){}
    operator hasync() const { return m_hasync; }
private:
    hasync m_hasync;
};

template<class ResultType>
class AsyncManager
{
public:
    typedef ResultType result_t;
    typedef std::future<result_t> future_t;
    typedef AsyncHandle<result_t> handle_t;

    static AsyncManager& getInstance()
    {
        static AsyncManager s_inst;
        return s_inst;
    }

    handle_t push(future_t& v)
    {
        handle_t ret = 0;
        if (m_handles.empty()) {
            ret = (handle_t)m_futures.size();
            m_futures.emplace_back(std::move(v));
        }
        else {
            ret = m_handles.back();
            m_futures[ret] = std::move(v);
            m_handles.pop_back();
        }
        return ret;
    }

    future_t pull(handle_t h)
    {
        if (h >= m_futures.size()) { return future_t(); }

        auto tmp = std::move(m_futures[h]);
        m_handles.push_back(h);
        return tmp;
    }

private:
    AsyncManager()
    {
        // 0th element is "null"
        m_futures.push_back(future_t());
    }
    ~AsyncManager() {}

    std::deque<future_t> m_futures;
    std::vector<hasync> m_handles;
};

template<class Task>
inline auto AsyncBegin(const Task& t) -> AsyncHandle<decltype(t())>
{
    auto f = std::async(std::launch::async, t);
    return AsyncManager<decltype(t())>::getInstance().push(f);
}

template<class ResultType>
inline ResultType AsyncEnd(AsyncHandle<ResultType> h)
{
    ResultType ret = ResultType();
    if (h == 0) { return ret; }

    auto f = AsyncManager<ResultType>::getInstance().pull(h);
    if (f.valid()) {
        ret = f.get();
    }
    return ret;
}

template<class ResultType>
inline ResultType AsyncEnd(hasync h)
{
    return AsyncEnd(AsyncHandle<ResultType>(h));
}

} // namespace usdi
