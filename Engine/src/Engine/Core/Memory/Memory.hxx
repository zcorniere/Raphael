#pragma once

struct Memory {
    static void* Malloc(uint32 Size, uint32 Alignment = 0);
    static void* Realloc(void* Original, uint32 Size, uint32 Alignment = 0);

    static void Free(void* Ptr);

    static bool GetAllocationSize(void* Ptr, uint32& OutSize);
    static const char* GetAllocatorName();
};

/// Allocator Interface
class Malloc
{
public:
    virtual void* Alloc(uint32 Size, uint32 Alignment = 0) = 0;
    virtual void* Realloc(void* Original, uint32 Size, uint32 Alignment = 0) = 0;

    virtual void Free(void* Ptr) = 0;

    virtual bool GetAllocationSize(void* Ptr, uint32& OutSize) = 0;
    virtual const char* GetAllocatorName() const = 0;

    virtual bool SupportPoison() const = 0;
};

namespace Raphael
{

/// @brief STL-compliant allocator
/// @tparam T The type of the allocated memory
template <typename T>
class Allocator
{
public:
    using value_type = T;

    constexpr Allocator() = default;
    constexpr ~Allocator() = default;
    constexpr Allocator(const Allocator&) noexcept = default;
    constexpr Allocator(Allocator&&) noexcept = default;

    template <class U>
    constexpr Allocator(const Allocator<U>&) noexcept
    {
    }

    /// @brief Return new allocated memory location of (sizeof(T) * Size)
    /// @param Size The amount of the T we want to allocate
    /// @param hint (unused)
    /// @return return a valid memory location
    T* allocate(std::size_t Size, const void* hint = nullptr)
    {
        (void)hint;
        if (Size == 0)
            return nullptr;
        if (Size > std::numeric_limits<std::size_t>::max() / sizeof(T))
            throw std::bad_array_new_length();

        if (T* p = static_cast<T*>(Memory::Malloc(Size * sizeof(T)))) {
            return p;
        }

        throw std::bad_alloc();
    }

    /// @brief Deallocate the given memory pointer
    /// @param pPointer the memory location to release
    /// @param n (unused)
    void deallocate(T* pPointer, std::size_t n = 0) noexcept
    {
        (void)n;
        if (pPointer)
            Memory::Free(pPointer);
    }

    bool operator==(const Allocator&) const
    {
        return true;
    }
};

}    // namespace Raphael

void* operator new(std::size_t n);
void* operator new[](std::size_t n);

void operator delete(void* p) noexcept;
void operator delete(void* p, std::size_t n) noexcept;
void operator delete[](void* p) noexcept;
void operator delete[](void* p, std::size_t n) noexcept;
