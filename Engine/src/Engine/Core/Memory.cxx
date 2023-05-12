#include "Engine/Core/Memory.hxx"

#if TRACY_ENABLE
    #include "tracy/Tracy.hpp"
#endif

static Raphael::Allocator<uint8> s_Allocator;

void *operator new(std::size_t n)
{
    void *Memory = s_Allocator.Allocate(n);

#if TRACY_ENABLE
    TracyAlloc(Memory, n);
#endif
    return Memory;
}

void *operator new[](std::size_t n)
{
    void *Memory = s_Allocator.Allocate(n);

#if TRACY_ENABLE
    TracyAlloc(Memory, n);
#endif
    return Memory;
}

void operator delete(void *p) noexcept
{
    if (p == nullptr) return;
#if TRACY_ENABLE
    TracyFree(p);
#endif
    s_Allocator.Free((uint8 *)p);
}

void operator delete(void *p, std::size_t n) noexcept
{
    (void)n;
    ::operator delete(p);
}

void operator delete[](void *p) noexcept
{
    if (p == nullptr) return;
#if TRACY_ENABLE
    TracyFree(p);
#endif

    s_Allocator.Free((uint8 *)p);
}

void operator delete[](void *p, std::size_t n) noexcept
{
    (void)n;
    ::operator delete[](p);
}
