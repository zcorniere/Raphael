#pragma once

#include <atomic>

namespace Raphael
{

namespace RefUtils
{
    void AddToLiveReferences(void *instance);
    void RemoveFromLiveReferences(void *instance);
    bool IsLive(void *instance);
}    // namespace RefUtils

class RefCounted
{
public:
    void IncRefCount() const
    {
        ++m_RefCount;
    }
    void DecRefCount() const
    {
        --m_RefCount;
    }

    std::uint32_t GetRefCount() const
    {
        return m_RefCount.load();
    }

private:
    mutable std::atomic<std::uint32_t> m_RefCount = 0;
};

template <typename T>
requires std::is_base_of_v<RefCounted, T>
class Ref
{
public:
    static Ref<T> CopyWithoutIncrement(const Ref<T> &other)
    {
        Ref<T> Result(nullptr);
        Result->m_ObjPtr = other.m_ObjPtr;
        return Result;
    }

public:
    Ref(): m_ObjPtr(nullptr)
    {
    }
    Ref(std::nullptr_t n): m_ObjPtr(nullptr)
    {
    }
    Ref(T *Object): m_ObjPtr(Object)
    {
        IncRef();
    }

    Ref(const Ref<T> &other): m_ObjPtr(other.m_ObjPtr)
    {
        IncRef();
    }

    template <typename Other>
    Ref(const Ref<Other> &other)
    {
        m_ObjPtr = (T *)other.m_ObjPtr;
        IncRef();
    }

    template <typename Other>
    Ref(const Ref<Other> &&other)
    {
        m_ObjPtr = (T *)other.m_ObjPtr;
        IncRef();
    }

    ~Ref()
    {
        DecRef();
    }

    Ref &operator=(std::nullptr_t)
    {
        DecRef();
        m_ObjPtr = nullptr;
        return *this;
    }

    Ref &operator=(const Ref<T> &other)
    {
        other.IncRef();
        DecRef();

        m_ObjPtr = other.m_ObjPtr;
        return *this;
    }

    template <typename Other>
    Ref &operator=(const Ref<Other> &other)
    {
        other.IncRef();
        DecRef();

        m_ObjPtr = other.m_ObjPtr;
        return *this;
    }

    template <typename Other>
    Ref &operator=(Ref<Other> &&other)
    {
        DecRef();

        m_ObjPtr = other.m_ObjPtr;
        other.m_ObjPtr = nullptr;
        return *this;
    }

    operator bool()
    {
        return m_ObjPtr != nullptr;
    }
    operator bool() const
    {
        return m_ObjPtr != nullptr;
    }

    T *operator->()
    {
        return m_ObjPtr;
    }
    const T *operator->() const
    {
        return m_ObjPtr;
    }

    T &operator*()
    {
        return *m_ObjPtr;
    }
    const T &operator*() const
    {
        return *m_ObjPtr;
    }

    T *Raw()
    {
        return m_ObjPtr;
    }
    const T *Raw() const
    {
        return m_ObjPtr;
    }

    void Reset(T *instance = nullptr)
    {
        DecRef();
        m_ObjPtr = instance;
    }

    template <typename Other>
    Ref<Other> As() const
    {
        return Ref<Other>(*this);
    }

    template <typename... Args>
    static Ref<T> Create(Args &&...args)
    {
        return Ref<T>(new T(std::forward<Args>(args)...));
    }

    bool operator==(const Ref<T> &other) const
    {
        return m_ObjPtr == other.m_ObjPtr;
    }

    bool operator!=(const Ref<T> &other) const
    {
        return !(*this == other);
    }

    bool EqualsObject(const Ref<T> &other)
    {
        if (!m_ObjPtr || !other.m_ObjPtr) return false;

        return *m_ObjPtr == *other.m_ObjPtr;
    }

private:
    void IncRef() const
    {
        if (m_ObjPtr) {
            m_ObjPtr->IncRefCount();
            RefUtils::AddToLiveReferences((void *)m_ObjPtr);
        }
    }

    void DecRef() const
    {
        if (m_ObjPtr) {
            m_ObjPtr->DecRefCount();
            if (m_ObjPtr->GetRefCount() == 0) {
                RefUtils::RemoveFromLiveReferences((void *)m_ObjPtr);
                delete m_ObjPtr;
                m_ObjPtr = nullptr;
            }
        }
    }

private:
    mutable T *m_ObjPtr = nullptr;

    template <class Other>
    friend class Ref;
};

}    // namespace Raphael
