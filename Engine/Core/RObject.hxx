#pragma once

#include <atomic>

namespace Raphael
{

namespace RObjectUtils
{
    void AddToLiveReferences(void *instance);
    void RemoveFromLiveReferences(void *instance);
    bool IsLive(void *instance);
}    // namespace RefUtils

class RObject
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
requires std::is_base_of_v<RObject, T>
class Ref
{
public:
    template <typename... Args>
    static Ref<T> Create(Args &&...args)
    {
        return Ref<T>(new T(std::forward<Args>(args)...));
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
        IncrementRefCount();
    }

    Ref(const Ref<T> &other): m_ObjPtr(other.m_ObjPtr)
    {
        IncrementRefCount();
    }

    template <typename Other>
    Ref(const Ref<Other> &other)
    {
        m_ObjPtr = (T *)other.m_ObjPtr;
        IncrementRefCount();
    }

    template <typename Other>
    Ref(const Ref<Other> &&other)
    {
        m_ObjPtr = (T *)other.m_ObjPtr;
        IncrementRefCount();
    }

    ~Ref()
    {
        DecrementRefCount();
    }

    Ref &operator=(std::nullptr_t)
    {
        DecrementRefCount();
        m_ObjPtr = nullptr;
        return *this;
    }

    Ref &operator=(const Ref<T> &other)
    {
        other.IncrementRefCount();
        DecrementRefCount();

        m_ObjPtr = other.m_ObjPtr;
        return *this;
    }

    template <typename Other>
    Ref &operator=(const Ref<Other> &other)
    {
        other.IncrementRefCount();
        DecrementRefCount();

        m_ObjPtr = other.m_ObjPtr;
        return *this;
    }

    template <typename Other>
    Ref &operator=(Ref<Other> &&other)
    {
        DecrementRefCount();

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
        DecrementRefCount();
        m_ObjPtr = instance;
    }

    template <typename Other>
    Ref<Other> As() const
    {
        return Ref<Other>(*this);
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
    void IncrementRefCount() const
    {
        if (m_ObjPtr) {
            m_ObjPtr->IncrementRefCount();
            RObjectUtils::AddToLiveReferences((void *)m_ObjPtr);
        }
    }

    void DecrementRefCount() const
    {
        if (m_ObjPtr) {
            m_ObjPtr->DecrementRefCount();
            if (m_ObjPtr->GetRefCount() == 0) {
                RObjectUtils::RemoveFromLiveReferences((void *)m_ObjPtr);
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
