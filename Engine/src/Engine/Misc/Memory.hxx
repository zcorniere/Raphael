#pragma once

namespace Raphael
{

/// @brief Malloc-based allocator
/// @tparam T The type of the allocated memory
template <typename T>
class Allocator
{
public:
    using value_type = T;

    Allocator() = default;

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

        if (T* p = static_cast<T*>(std::malloc(Size * sizeof(T)))) {
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
            std::free(pPointer);
    }
};

}    // namespace Raphael

void* operator new(std::size_t n);
void* operator new[](std::size_t n);

void operator delete(void* p) noexcept;
void operator delete(void* p, std::size_t n) noexcept;
void operator delete[](void* p) noexcept;
void operator delete[](void* p, std::size_t n) noexcept;
