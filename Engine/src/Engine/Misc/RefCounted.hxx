#pragma once

/// Custom Ref Counting class
class RefCounted
{
public:
    virtual ~RefCounted()
    {
    }

    /// Increment the ref count of the RObject
    void AddRef() const
    {
        checkMsg(m_RefCount <= UINT32_MAX - 1, "Ref count have overflowed !");
        m_RefCount.fetch_add(1, std::memory_order_acq_rel);
    }
    /// Decrement the ref count of the RObject
    virtual uint32 Release() const
    {
        checkMsg(m_RefCount > 0, "Ref count is already at 0");
        m_RefCount.fetch_sub(1, std::memory_order_acq_rel);
        if (m_RefCount == 0) {
            Destroy();
        }
        return m_RefCount;
    }

    /// Get the current ref count
    std::uint32_t GetRefCount() const
    {
        return m_RefCount.load();
    }

protected:
    virtual void Destroy() const
    {
    }

private:
    // We are obviously refereced at lease once when we are created
    mutable std::atomic<std::uint32_t> m_RefCount = 1;
};

/**
 * A smart pointer to an object which implements AddRef/Release.
 */
template <typename ReferencedType>
class TRefCountPtr
{
public:
    FORCEINLINE TRefCountPtr(): Reference(nullptr)
    {
    }

    TRefCountPtr(ReferencedType* InReference, bool bAddRef = true)
    {
        Reference = InReference;
        if (Reference && bAddRef) {
            Reference->AddRef();
        }
    }

    TRefCountPtr(const TRefCountPtr& Copy)
    {
        Reference = Copy.Reference;
        if (Reference) {
            Reference->AddRef();
        }
    }

    template <typename CopyReferencedType>
    explicit TRefCountPtr(const TRefCountPtr<CopyReferencedType>& Copy)
    {
        Reference = static_cast<ReferencedType*>(Copy.GetReference());
        if (Reference) {
            Reference->AddRef();
        }
    }

    FORCEINLINE TRefCountPtr(TRefCountPtr&& Move)
    {
        Reference = Move.Reference;
        Move.Reference = nullptr;
    }

    template <typename MoveReferencedType>
    explicit TRefCountPtr(TRefCountPtr<MoveReferencedType>&& Move)
    {
        Reference = static_cast<ReferencedType*>(Move.GetReference());
        Move.Reference = nullptr;
    }

    ~TRefCountPtr()
    {
        if (Reference) {
            Reference->Release();
        }
    }

    TRefCountPtr& operator=(ReferencedType* InReference)
    {
        if (Reference != InReference) {
            ReferencedType* OldReference = Reference;
            Reference = InReference;
            if (Reference) {
                Reference->AddRef();
            }
            if (OldReference) {
                OldReference->Release();
            }
        }
        return *this;
    }

    FORCEINLINE TRefCountPtr& operator=(const TRefCountPtr& InPtr)
    {
        return *this = InPtr.Reference;
    }

    template <typename CopyReferencedType>
    FORCEINLINE TRefCountPtr& operator=(const TRefCountPtr<CopyReferencedType>& InPtr)
    {
        return *this = InPtr.GetReference();
    }

    TRefCountPtr& operator=(TRefCountPtr&& InPtr)
    {
        if (this != &InPtr) {
            ReferencedType* OldReference = Reference;
            Reference = InPtr.Reference;
            InPtr.Reference = nullptr;
            if (OldReference) {
                OldReference->Release();
            }
        }
        return *this;
    }

    template <typename MoveReferencedType>
    TRefCountPtr& operator=(TRefCountPtr<MoveReferencedType>&& InPtr)
    {
        ReferencedType* OldReference = Reference;
        Reference = InPtr.Reference;
        InPtr.Reference = nullptr;
        if (OldReference) {
            OldReference->Release();
        }
        return *this;
    }

    FORCEINLINE ReferencedType* operator->() const
    {
        return Reference;
    }

    FORCEINLINE operator ReferencedType*() const
    {
        return Reference;
    }

    FORCEINLINE ReferencedType* GetReference() const
    {
        return Reference;
    }

    FORCEINLINE bool IsValid() const
    {
        return Reference != nullptr;
    }

    FORCEINLINE void SafeRelease()
    {
        *this = nullptr;
    }

    uint32 GetRefCount()
    {
        uint32 Result = 0;
        if (Reference) {
            Result = Reference->GetRefCount();
            check(Result > 0);
        }
        return Result;
    }

    FORCEINLINE void Swap(TRefCountPtr& InPtr)
    {
        ReferencedType* OldReference = Reference;
        Reference = InPtr.Reference;
        InPtr.Reference = OldReference;
    }

private:
    ReferencedType* Reference;

    template <typename OtherType>
    friend class TRefCountPtr;

public:
    FORCEINLINE bool operator==(const TRefCountPtr& B) const
    {
        return GetReference() == B.GetReference();
    }

    FORCEINLINE bool operator==(ReferencedType* B) const
    {
        return GetReference() == B;
    }
};
