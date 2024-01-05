#pragma once

/// Smart unique pointer that will get free the memory when destroyed
#include "Engine/Compilers/ClangCompiler.hxx"
#include <cstddef>

template <typename Type>
class UniquePtr
{
public:
    /// Store the given pointer and take ownership of it. The pointer may be eventually destroyed using delete
    FORCEINLINE UniquePtr(Type* ptr): InternalPtr(ptr)
    {
    }
    /// Store the given pointer and take ownership of it. The use must provide a function to be called when destroying
    /// the pointer
    FORCEINLINE UniquePtr(Type* ptr, std::function<void(Type*)>&& InDeletor): InternalPtr(ptr), Deletor(InDeletor)
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

    /// Destroy owned pointer and replace it by the provided pointer
    ///
    /// @param ReplacementPtr The new value of the internal pointer, default to nullptr
    FORCEINLINE void Reset(Type* const ReplacementPtr = nullptr)
    {
        if (InternalPtr != nullptr) {
            Deletor(InternalPtr);
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

    /// Check for the validity of the internal pointer
    FORCEINLINE explicit operator bool() const
    {
        return InternalPtr != nullptr;
    }

private:
    Type* InternalPtr = nullptr;
    const std::function<void(Type*)> Deletor = [](Type* Ptr) { delete Ptr; };
};

template <typename Type, typename... ArgsType>
/// Make a new pointer of Type Type using ArgsType as a ctor argument, and return a Unique Ptr of it
UniquePtr<Type> MakeUnique(ArgsType&&... Args)
{
    return UniquePtr<Type>(new Type(Args...));
}

template <typename Type>
class UniqueCPtr : public UniquePtr<Type>
{
public:
    FORCEINLINE UniqueCPtr(Type* ptr)
        : UniquePtr<Type>(ptr, [](Type* Ptr) { std::free(const_cast<std::remove_const_t<Type>*>(Ptr)); })
    {
    }
};
