#pragma once

namespace Raphael
{

template <typename T>
class Allocator
{
public:
    using value_type = T;

    Allocator() = default;

    T* Allocate(std::size_t Size)
    {
        if (Size == 0)
            return nullptr;
        if (Size > std::numeric_limits<std::size_t>::max() / sizeof(T))
            throw std::bad_array_new_length();

        if (T* p = static_cast<T*>(std::malloc(Size * sizeof(T)))) {
            return p;
        }

        throw std::bad_alloc();
    }

    void Free(T* pPointer) noexcept
    {
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
