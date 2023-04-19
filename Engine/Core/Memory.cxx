#include "Engine/Core/Memory.hxx"

#if TRACY_ENABLE
    #include "tracy/Tracy.hpp"
#endif

static Raphael::Allocator<unsigned char> s_Allocator;

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

void operator delete(void *p)
{
    if (p == nullptr) return;
#if TRACY_ENABLE
    TracyFree(p);
#endif
    s_Allocator.Free((unsigned char *)p, 0);
}

void operator delete[](void *p)
{
    if (p == nullptr) return;
#if TRACY_ENABLE
    TracyFree(p);
#endif

    s_Allocator.Free((unsigned char *)p, 0);
}
