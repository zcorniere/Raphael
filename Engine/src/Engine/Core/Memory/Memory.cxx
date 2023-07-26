#include "Engine/Core/Memory/Memory.hxx"

#include "Engine/Core/Memory/AllocatorPoison.hxx"
#include "Engine/Platforms/PlatformMisc.hxx"

Malloc* GMalloc = 0;

static void EnsureAllocatorIsSetup()
{
    if (UNLIKELY(!GMalloc)) {
        // must manually allocate the memory
        GMalloc = PlatformMisc::BaseAllocator();

#if RPH_POISON_ALLOCATION
        void* AllocPoisonMemory = std::malloc(sizeof(AllocatorPoison));
        GMalloc = new (AllocPoisonMemory) AllocatorPoison(GMalloc);
#endif
    }
}

void* Memory::Malloc(uint32 Size, uint32 Alignment)
{
    EnsureAllocatorIsSetup();

    void* Memory = GMalloc->Alloc(Size, Alignment);
    RPH_PROFILE_ALLOC(Memory, Size);
    return Memory;
}
void* Memory::Realloc(void* Original, uint32 Size, uint32 Alignment)
{
    EnsureAllocatorIsSetup();

    RPH_PROFILE_FREE(Original);
    void* Memory = GMalloc->Realloc(Original, Size, Alignment);
    RPH_PROFILE_ALLOC(Memory, Size);
    return Memory;
}

void Memory::Free(void* Ptr)
{
    EnsureAllocatorIsSetup();
    RPH_PROFILE_FREE(Ptr);

    return GMalloc->Free(Ptr);
}

bool Memory::GetAllocationSize(void* Ptr, uint32& OutSize)
{
    EnsureAllocatorIsSetup();
    return GMalloc->GetAllocationSize(Ptr, OutSize);
}

const char* Memory::GetAllocatorName()
{
    EnsureAllocatorIsSetup();
    return GMalloc->GetAllocatorName();
}

void* operator new(std::size_t n)
{
    return Memory::Malloc(n);
}

void* operator new[](std::size_t n)
{
    return Memory::Malloc(n);
}

void operator delete(void* p) noexcept
{
    if (p == nullptr)
        return;

    RPH_PROFILE_FREE(p);
    Memory::Free((uint8*)p);
}

void operator delete(void* p, std::size_t n) noexcept
{
    (void)n;
    if (p == nullptr)
        return;

    RPH_PROFILE_FREE(p);
    Memory::Free(p);
}

void operator delete[](void* p) noexcept
{
    if (p == nullptr)
        return;

    RPH_PROFILE_FREE(p);
    Memory::Free(p);
}

void operator delete[](void* p, std::size_t n) noexcept
{
    (void)n;
    if (p == nullptr)
        return;

    RPH_PROFILE_FREE(p);
    Memory::Free(p);
}
