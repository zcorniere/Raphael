#pragma once

/// Smart unique pointer that will get free the memory when destroyed
#include <cstddef>

template <typename Type>
class UniquePtr
{
public:
    /// Create a new pointer, and take ownership of it
    FORCEINLINE UniquePtr(Type* ptr): InternalPtr(ptr)
    {
    }
    FORCEINLINE UniquePtr(std::nullptr_t): InternalPtr(nullptr)
    {
    }

    FORCEINLINE ~UniquePtr()
    {
        if (InternalPtr) {
            Reset();
        }
    }

    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr& other) = delete;
    UniquePtr& operator=(std::nullptr_t)
    {
        Reset();
        return *this;
    }

    UniquePtr(UniquePtr&& Other): UniquePtr(Other.Release())
    {
    }
    template <typename OtherType>
    requires std::convertible_to<OtherType, Type> || std::derived_from<OtherType, Type>
    UniquePtr(UniquePtr<OtherType>&& Other): UniquePtr(Other.Release())
    {
    }
    UniquePtr& operator=(UniquePtr&& other)
    {
        Release(other.Release(InternalPtr));
        return *this;
    }

    /// Release ownership of the internal pointer and return it
    ///
    /// @param ReplacementPtr The new value of the internal pointer, default to nullptr
    FORCEINLINE Type* Release(Type* const ReplacementPtr = nullptr)
    {
        Type* const ReleasePtr = InternalPtr;
        InternalPtr = ReplacementPtr;
        return ReleasePtr;
    }

    /// Delete owned poiter and replace it by the provided pointer
    ///
    /// @param ReplacementPtr The new value of the internal pointer, default to nullptr
    FORCEINLINE void Reset(Type* const ReplacementPtr = nullptr)
    {
        if (InternalPtr != nullptr) {
            delete InternalPtr;
        }
        InternalPtr = ReplacementPtr;
    }

    /// Return the internal pointer, keep ownership
    FORCEINLINE Type* Get() const
    {
        return InternalPtr;
    }

    FORCEINLINE Type* operator->() const
    {
        return InternalPtr;
    }
    FORCEINLINE Type& operator*() const
    {
        return *InternalPtr;
    }

    FORCEINLINE explicit operator bool() const
    {
        return InternalPtr != nullptr;
    }

private:
    Type* InternalPtr = nullptr;
};

template <typename Type, typename... ArgsType>
UniquePtr<Type> MakeUnique(ArgsType&&... Args)
{
    return UniquePtr<Type>(new Type(Args...));
}
