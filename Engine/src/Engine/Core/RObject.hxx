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

/// Mark the RObject as a live ref
void AddToLiveReferences(RObject* instance);
/// Mark the RObject as dead
void RemoveFromLiveReferences(RObject* instance);
/// Is the RObject live ?
bool IsLive(RObject* instance);

/// @brief check is there is any live RObject
/// @return true is a RObject was not deleted
bool AreThereAnyLiveObject(bool bPrintObjects = true);

}    // namespace RObjectUtils

class FNamedClass : public RTTI::FEnable
{
    RTTI_DECLARE_TYPEINFO(FNamedClass);

public:
    virtual ~FNamedClass() = default;

    /// Give the object a debug name
    virtual void SetName(std::string_view InName)
    {
        m_Name = InName;
    }

    /// Return the debug name of the object
    const std::string& GetName() const
    {
        return m_Name;
    }

    /// Return a string representing the Object
    virtual std::string ToString() const
    {
        return std::format("(\"{:s}\" <{:s}> {:p})", GetName(), GetTypeName(), (void*)this);
    }

private:
    std::string m_Name = "Unnamed";
};

/// Custom Ref Counting class
class RObject : public FNamedClass
{
    RTTI_DECLARE_TYPEINFO(RObject, FNamedClass);

public:
    virtual ~RObject()
    {
    }

    /// Override this function to be able to override the behaviour of Ref::IsValid;
    virtual bool IsValid() const
    {
        return true;
    }

    /// Get the current ref count
    std::uint32_t GetRefCount() const
    {
        return m_RefCount.load(std::memory_order_relaxed);
    }

private:
    /// Increment the ref count of the RObject
    void IncrementRefCount() const
    {
        checkMsg(m_RefCount <= UINT32_MAX - 1, "Ref count have overflowed !");
        m_RefCount.fetch_add(1, std::memory_order_acq_rel);
    }
    /// Decrement the ref count of the RObject
    void DecrementRefCount() const
    {
        if (!ensureAlwaysMsg(m_RefCount > 0, "Ref count is already at 0")) {
            return;
        }
        m_RefCount.fetch_sub(1, std::memory_order_acq_rel);
    }

private:
    mutable std::atomic<std::uint32_t> m_RefCount = 0;

    // Allow Refs to access private members
    template <class Other>
    friend class Ref;
};

/// @brief Hold a reference to a RObject
/// @tparam T The type contained by the Ref (MUST BE A ROBJECT)
template <typename T>
class Ref
{
public:
    /// @brief Create a new RObject and give it is name
    /// @return the new RObject
    template <typename... Args>
    FORCEINLINE static Ref<T> CreateNamed(std::string_view Name, Args&&... args)
    {
        Ref<T> NewRef = CreateInternal(std::forward<Args>(args)...);
        NewRef->SetName(Name);
        return NewRef;
    }

    /// @brief Create a new RObject
    /// @return the new RObject
    template <typename... Args>
    FORCEINLINE static Ref<T> Create(Args&&... args)
    {
        return CreateInternal(std::forward<Args>(args)...);
    }

private:
    template <typename... Args>
    FORCEINLINE static Ref<T> CreateInternal(Args&&... args)
    {
        T* const NewRef = new T(std::forward<Args>(args)...);
        LOG(RObjectUtils::LogRObject, Trace, "Creating RObject {:s}", NewRef->ToString());

        return Ref<T>(NewRef);
    }

public:
    Ref(): m_ObjPtr(nullptr)
    {
    }

    Ref(std::nullptr_t): m_ObjPtr(nullptr)
    {
    }

    Ref(T* Object): m_ObjPtr(Object)
    {
        static_assert(std::is_base_of<RObject, T>::value, "Class is not RefCounted!");

        IncrementRefCount();
    }

    explicit Ref(WeakRef<T>& other): Ref(other.m_Instance)
    {
    }

    Ref(const Ref<T>& other): Ref(other.m_ObjPtr)
    {
    }

    template <typename Other>
    Ref(const Ref<Other>& other): Ref((T*)other.m_ObjPtr)
    {
    }

    template <typename Other>
    Ref(const Ref<Other>&& other)
    {
        m_ObjPtr = (T*)other.m_ObjPtr;
        other.m_ObjPtr = nullptr;
    }

    ~Ref()
    {
        DecrementRefCount();
    }

    Ref& operator=(std::nullptr_t)
    {
        DecrementRefCount();
        m_ObjPtr = nullptr;
        return *this;
    }

    Ref& operator=(const Ref<T>& other)
    {
        other.IncrementRefCount();
        DecrementRefCount();

        m_ObjPtr = other.m_ObjPtr;
        return *this;
    }

    template <typename Other>
    Ref& operator=(const Ref<Other>& other)
    {
        other.IncrementRefCount();
        DecrementRefCount();

        m_ObjPtr = (const T*)other.m_ObjPtr;
        return *this;
    }

    template <typename Other>
    Ref& operator=(Ref<Other>&& other)
    {
        DecrementRefCount();

        m_ObjPtr = (T*)other.m_ObjPtr;
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

    T* operator->()
    {
        checkSlow(IsValid());
        return m_ObjPtr;
    }
    const T* operator->() const
    {
        checkSlow(IsValid());
        return m_ObjPtr;
    }

    T& operator*()
    {
        checkSlow(IsValid());
        return *m_ObjPtr;
    }
    const T& operator*() const
    {
        checkSlow(IsValid());
        return *m_ObjPtr;
    }

    T* Raw()
    {
        return m_ObjPtr;
    }
    const T* Raw() const
    {
        return m_ObjPtr;
    }

    void Reset(T* instance = nullptr)
    {
        DecrementRefCount();
        m_ObjPtr = instance;
    }

    template <typename Other>
    requires std::convertible_to<T*, Other*> || std::derived_from<Other, T>
    Ref<Other> As() const
    {
        return Ref<Other>(*this);
    }

    template <typename Other>
    requires std::convertible_to<T*, Other*> || std::derived_from<Other, T>
    const Other* AsRaw() const
    {
        return Raw()->template Cast<const Other>();
    }

    template <typename Other>
    requires std::convertible_to<T*, Other*> || std::derived_from<Other, T>
    Other* AsRaw()
    {
        return Raw()->template Cast<Other>();
    }

    bool operator==(const Ref<T>& other) const
    {
        return m_ObjPtr == other.m_ObjPtr;
    }

    bool EqualsObject(const Ref<T>& other)
    {
        if (!m_ObjPtr || !other.m_ObjPtr)
            return false;

        return *m_ObjPtr == *other.m_ObjPtr;
    }

    bool IsValid() const
    {
        return m_ObjPtr ? (RObjectUtils::IsLive(m_ObjPtr) && m_ObjPtr->IsValid()) : (false);
    }

private:
    void IncrementRefCount() const
    {
        static_assert(std::is_base_of<RObject, T>::value, "Class is not RefCounted!");

        if (!m_ObjPtr)
            return;

        m_ObjPtr->IncrementRefCount();
        LOG(RObjectUtils::LogRObject, Trace, "Increment RObject {:s} refcount: {}", m_ObjPtr->ToString(),
            m_ObjPtr->GetRefCount());
        RObjectUtils::AddToLiveReferences(m_ObjPtr);
    }

    void DecrementRefCount() const
    {
        static_assert(std::is_base_of<RObject, T>::value, "Class is not RefCounted!");

        if (!m_ObjPtr)
            return;

        m_ObjPtr->DecrementRefCount();
        LOG(RObjectUtils::LogRObject, Trace, "Decrement RObject {:s} refcount: {}", m_ObjPtr->ToString(),
            m_ObjPtr->GetRefCount());

        if (m_ObjPtr->GetRefCount() > 0)
            return;

        LOG(RObjectUtils::LogRObject, Trace, "Deleting RObject {:s}", m_ObjPtr->ToString());

        delete m_ObjPtr;
        RObjectUtils::RemoveFromLiveReferences(m_ObjPtr);
        m_ObjPtr = nullptr;
    }

private:
    mutable T* m_ObjPtr = nullptr;

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
    ~WeakRef() = default;

    WeakRef(Ref<T> ref)
    {
        m_Instance = ref.Raw();
    }

    WeakRef(T* instance)
    {
        m_Instance = instance;
    }

    T* operator->()
    {
        check(IsValid());
        return m_Instance;
    }
    const T* operator->() const
    {
        check(IsValid());
        return m_Instance;
    }

    T& operator*()
    {
        check(IsValid());
        return *m_Instance;
    }
    const T& operator*() const
    {
        check(IsValid());
        return *m_Instance;
    }

    Ref<T> Pin() const
    {
        check(IsValid());
        return Ref(m_Instance);
    }

    bool IsValid() const
    {
        return m_Instance ? (RObjectUtils::IsLive(m_Instance) && m_Instance->IsValid()) : (false);
    }
    T* Raw()
    {
        return m_Instance;
    }
    const T* Raw() const
    {
        return m_Instance;
    }
    operator bool() const
    {
        return IsValid();
    }
    operator T*() const
    {
        return m_Instance;
    }

    bool operator==(const Ref<T>& other) const
    {
        return IsValid() && m_Instance == other.m_ObjPtr;
    }

    bool operator==(const WeakRef<T>& other) const
    {
        return IsValid() && other.IsValid() && m_Instance == other.m_Instance;
    }

private:
    T* m_Instance = nullptr;

    template <typename Other>
    friend class Ref;
};

namespace std
{
template <typename T>
struct hash<Ref<T>> {
    std::size_t operator()(const Ref<T>& ref) const
    {
        return std::hash<const T*>{}(ref.Raw());
    }
};

template <typename T>
struct hash<WeakRef<T>> {
    std::size_t operator()(const WeakRef<T>& ref) const
    {
        return std::hash<const T*>{}(ref.Raw());
    }
};

}    // namespace std
