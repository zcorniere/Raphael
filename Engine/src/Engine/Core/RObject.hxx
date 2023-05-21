#pragma once

#include <atomic>

#include <Engine/Misc/Assertions.hxx>

template <typename T>
class Ref;
template <typename T>
class WeakRef;
class RObject;

namespace RObjectUtils
{

DECLARE_LOGGER_CATEGORY(Core, LogRObject, Warning)

void AddToLiveReferences(RObject *instance);
void RemoveFromLiveReferences(RObject *instance);
bool IsLive(RObject *instance);

bool AreThereAnyLiveObject(bool bPrintObjects = true);

}    // namespace RObjectUtils

class RObject
{
public:
    virtual ~RObject()
    {
    }

    virtual void SetName(std::string_view InName)
    {
        m_Name = InName;
    }

    void SetTypeName(std::string_view InTypeName)
    {
        // Should not be called multiple time
        check(m_TypeName.empty());
        m_TypeName = InTypeName;
    }

    const std::string &GetTypeName() const { return m_TypeName; }

    const std::string &GetName() const
    {
        return m_Name;
    }

    virtual std::string ToString() const
    {
        if (GetName().empty()) {
            return std::format("(<{:s}> {:p})", GetTypeName(), (void *)this);
        } else {
            return std::format("(\"{:s}\" <{:s}> {:p})", GetName(), GetTypeName(), (void *)this);
        }
    }

    void IncrementRefCount() const
    {
        checkMsg(m_RefCount <= UINT32_MAX - 1, "Ref count have overflowed !");
        m_RefCount.fetch_add(1, std::memory_order_acq_rel);
    }
    void DecrementRefCount() const
    {
        if (!verifyAlwaysMsg(m_RefCount > 0, "Ref count is already at 0")) { return; }
        m_RefCount.fetch_sub(1, std::memory_order_acq_rel);
    }

    std::uint32_t GetRefCount() const
    {
        return m_RefCount.load();
    }

private:
    std::string m_Name;
    std::string m_TypeName;
    mutable std::atomic<std::uint32_t> m_RefCount = 0;
};

template <typename T>
class Ref
{
public:
    template <typename... Args>
    requires std::is_constructible_v<T, Args...>
    static Ref<T> CreateNamed(std::string_view Name, Args &&...args)
    {
        return CreateInternal(Name, std::forward<Args>(args)...);
    }

    template <typename... Args>
    requires std::is_constructible_v<T, Args...>
    static Ref<T> Create(Args &&...args)
    {
        return CreateInternal("", std::forward<Args>(args)...);
    }

private:
    template <typename... Args>
        requires std::is_constructible_v<T, Args...>
    static Ref<T> CreateInternal(std::string_view Name, Args &&...args)
    {
        T *ObjectPtr = new T(std::forward<Args>(args)...);
        ObjectPtr->SetTypeName(type_name<T>());
        ObjectPtr->SetName(Name);

        LOG(RObjectUtils::LogRObject, Trace, "Creating RObject {:s}", ObjectPtr->ToString());

        return Ref<T>(ObjectPtr);
    }

public:
    Ref(): m_ObjPtr(nullptr) {}

    Ref(std::nullptr_t): m_ObjPtr(nullptr)
    {
    }

    Ref(T *Object): m_ObjPtr(Object)
    {
        static_assert(std::is_base_of<RObject, T>::value, "Class is not RefCounted!");

        IncrementRefCount();
    }

    explicit Ref(WeakRef<T> &other): Ref(other.m_Instance) {}

    Ref(const Ref<T> &other): Ref(other.m_ObjPtr) {}

    template <typename Other>
    Ref(const Ref<Other> &other): Ref((T *)other.m_ObjPtr)
    {
    }

    template <typename Other>
    Ref(const Ref<Other> &&other)
    {
        m_ObjPtr = (T *)other.m_ObjPtr;
        other.m_ObjPtr = nullptr;
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

        m_ObjPtr = (const T *)other.m_ObjPtr;
        return *this;
    }

    template <typename Other>
    Ref &operator=(Ref<Other> &&other)
    {
        DecrementRefCount();

        m_ObjPtr = (T *)other.m_ObjPtr;
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

    bool EqualsObject(const Ref<T> &other)
    {
        if (!m_ObjPtr || !other.m_ObjPtr) return false;

        return *m_ObjPtr == *other.m_ObjPtr;
    }

private:
    void IncrementRefCount() const
    {
        static_assert(std::is_base_of<RObject, T>::value, "Class is not RefCounted!");

        if (!m_ObjPtr) return;

        m_ObjPtr->IncrementRefCount();
        RObjectUtils::AddToLiveReferences(m_ObjPtr);
    }

    void DecrementRefCount() const
    {
        static_assert(std::is_base_of<RObject, T>::value, "Class is not RefCounted!");

        if (!m_ObjPtr) return;

        m_ObjPtr->DecrementRefCount();

        if (m_ObjPtr->GetRefCount() > 0) return;

        LOG(RObjectUtils::LogRObject, Trace, "Deleting RObject {:s}", m_ObjPtr->ToString());

        delete m_ObjPtr;
        RObjectUtils::RemoveFromLiveReferences(m_ObjPtr);
        m_ObjPtr = nullptr;
    }

private:
    mutable T *m_ObjPtr = nullptr;

    template <class Other>
    friend class Ref;

    template <class Other>
    friend class WeakRef;
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

    template <typename Other>
    friend class Ref;
};