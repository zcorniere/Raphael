#include "Engine/Misc/Memory.hxx"

static Raphael::Allocator<uint8> s_Allocator;

void* operator new(std::size_t n)
{
    void* Memory = s_Allocator.allocate(n);

    RPH_PROFILE_ALLOC(Memory, n);
    return Memory;
}

void* operator new[](std::size_t n)
{
    void* Memory = s_Allocator.allocate(n);

    RPH_PROFILE_ALLOC(Memory, n);
    return Memory;
}

void operator delete(void* p) noexcept
{
    if (p == nullptr)
        return;

    RPH_PROFILE_FREE(p);
    s_Allocator.deallocate((uint8*)p);
}

void operator delete(void* p, std::size_t n) noexcept
{
    if (p == nullptr)
        return;

    RPH_PROFILE_FREE(p);
    s_Allocator.deallocate((uint8*)p, n);
}

void operator delete[](void* p) noexcept
{
    if (p == nullptr)
        return;

    RPH_PROFILE_FREE(p);
    s_Allocator.deallocate((uint8*)p);
}

void operator delete[](void* p, std::size_t n) noexcept
{
    if (p == nullptr)
        return;

    RPH_PROFILE_FREE(p);
    s_Allocator.deallocate((uint8*)p, n);
}
