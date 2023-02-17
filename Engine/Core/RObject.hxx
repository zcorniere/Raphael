#pragma once

#include <atomic>

#include <Engine/Misc/Assertions.hxx>

namespace RObjectUtils
{
DECLARE_LOGGER_CATEGORY(Core, LogRObject, Error);

void AddToLiveReferences(void *instance);
void RemoveFromLiveReferences(void *instance);
bool IsLive(void *instance);
}    // namespace RObjectUtils

class RObject
{
public:
    void IncrementRefCount() const
    {
        checkMsg(m_RefCount <= UINT32_MAX - 1, "Ref count have overflowed !");
        ++m_RefCount;
    }
    void DecrementRefCount() const
    {
        checkMsg(m_RefCount > 0, "Ref count is already at 0");
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
        static_assert(std::is_base_of<RObject, T>::value, "Class is not RefCounted!");

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
        check(m_ObjPtr);
        return m_ObjPtr;
    }
    const T *operator->() const
    {
        check(m_ObjPtr);
        return m_ObjPtr;
    }

    T &operator*()
    {
        check(m_ObjPtr);
        return *m_ObjPtr;
    }
    const T &operator*() const
    {
        check(m_ObjPtr);
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

template <typename T>
class WeakRef
{
public:
    WeakRef() = default;

    WeakRef(Ref<T> ref)
    {
        m_Instance = ref.Raw();
    }

    WeakRef(T *instance)
    {
        m_Instance = instance;
    }

    T *operator->()
    {
        check(IsValid());
        return m_Instance;
    }
    const T *operator->() const
    {
        check(IsValid());
        return m_Instance;
    }

    T &operator*()
    {
        check(IsValid());
        return *m_Instance;
    }
    const T &operator*() const
    {
        check(IsValid());
        return *m_Instance;
    }

    bool IsValid() const
    {
        return m_Instance ? RObjectUtils::IsLive(m_Instance) : false;
    }
    operator bool() const
    {
        return IsValid();
    }

private:
    T *m_Instance = nullptr;
};
