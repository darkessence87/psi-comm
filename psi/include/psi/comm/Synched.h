
#pragma once

#include <mutex>

namespace psi::comm {

/**
 * @brief Synched class is used for thread-safe access to wrapped object.
 * 
 * @tparam T type of object
 * @tparam M type of mutex
 */
template <typename T, typename M = std::recursive_mutex>
class Synched
{
public:
    struct Locker {
        Locker(T *const obj, M &mtx)
            : m_obj(obj)
            , m_lock(mtx)
        {
        }

        Locker(Locker &) = delete;
        Locker &operator=(Locker &) = delete;

        Locker(Locker &&locker)
            : m_obj(std::move(locker.m_obj))
            , m_lock(std::move(locker.m_lock))
        {
        }

        Locker &&operator=(Locker &&locker)
        {
            return std::move(locker);
        }

        ~Locker() {}

        T *operator->()
        {
            return m_obj;
        }

        const T *operator->() const
        {
            return m_obj;
        }

    private:
        T *const m_obj;
        std::unique_lock<M> m_lock;
    };

    Synched(std::shared_ptr<T> obj)
        : m_object(obj)
        , m_mutex(std::make_shared<M>())
    {
    }

    Locker operator->()
    {
        return Locker(m_object.get(), *m_mutex);
    }

    const Locker operator->() const
    {
        return Locker(m_object.get(), *m_mutex);
    }

private:
    std::shared_ptr<T> m_object;
    std::shared_ptr<M> m_mutex;
};

} // namespace psi::comm