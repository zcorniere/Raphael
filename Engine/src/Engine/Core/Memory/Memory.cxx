#include "Engine/Core/Memory/Memory.hxx"

#include "Engine/Core/Memory/AllocatorPoison.hxx"
#include "Engine/Misc/Assertions.hxx"
#include "Engine/Platforms/PlatformMisc.hxx"

IMallocInterface* GMalloc = 0;

static void EnsureAllocatorIsSetup()
{
    // Note: must manually allocate the memory
    if (!GMalloc) [[unlikely]] {
        checkNoReentry();

        static char AllocatorMemory[sizeof(IMallocInterface)];
        std::memset(AllocatorMemory, 0, sizeof(IMallocInterface));
        if (!FPlatformMisc::BaseAllocator(AllocatorMemory)) {
            checkNoEntry();
        }
        GMalloc = reinterpret_cast<IMallocInterface*>(AllocatorMemory);

#if RPH_POISON_ALLOCATION
        if (GMalloc->SupportPoison()) {
            static char PoisonAllocatorMemory[sizeof(FAllocatorPoison)];
            new (PoisonAllocatorMemory) FAllocatorPoison(GMalloc);
            GMalloc = reinterpret_cast<IMallocInterface*>(PoisonAllocatorMemory);
        }
#endif
    }
}

void* Memory::Malloc(uint32 Size, uint32 Alignment)
{
    EnsureAllocatorIsSetup();

    void* const Memory = GMalloc->Alloc(Size, Alignment);
    RPH_PROFILE_ALLOC(Memory, Size);
    return Memory;
}
void* Memory::Realloc(void* Original, uint32 Size, uint32 Alignment)
{
    EnsureAllocatorIsSetup();

    if (!Original) {
        RPH_PROFILE_FREE(Original);
    }
    void* const Memory = GMalloc->Realloc(Original, Size, Alignment);
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

void* operator new(std::size_t n, const std::nothrow_t& tag) noexcept
{
    (void)tag;
    return Memory::Malloc(n);
}

void* operator new(std::size_t n, std::align_val_t a)
{
    return Memory::Malloc(n, static_cast<uint32>(a));
}

void* operator new(std::size_t n, std::align_val_t a, const std::nothrow_t& tag) noexcept
{
    (void)tag;
    return Memory::Malloc(n, static_cast<uint32>(a));
}

void* operator new[](std::size_t n)
{
    return Memory::Malloc(n);
}

void* operator new[](std::size_t n, const std::nothrow_t& tag) noexcept
{
    (void)tag;
    return Memory::Malloc(n);
}

void* operator new[](std::size_t n, std::align_val_t a)
{
    return Memory::Malloc(n, static_cast<uint32>(a));
}

void* operator new[](std::size_t n, std::align_val_t a, const std::nothrow_t& tag) noexcept
{
    (void)tag;
    return Memory::Malloc(n, static_cast<uint32>(a));
}

void operator delete(void* p) noexcept
{
    if (p == nullptr)
        return;

    Memory::Free((uint8*)p);
}

void operator delete(void* p, std::size_t n) noexcept
{
    (void)n;
    if (p == nullptr)
        return;

    Memory::Free(p);
}

void operator delete(void* p, std::size_t n, std::align_val_t a) noexcept
{
    (void)n;
    (void)a;
    if (p == nullptr)
        return;

    Memory::Free(p);
}

void operator delete[](void* p) noexcept
{
    if (p == nullptr)
        return;

    Memory::Free(p);
}

void operator delete[](void* p, std::size_t n) noexcept
{
    (void)n;
    if (p == nullptr)
        return;

    Memory::Free(p);
}

void operator delete[](void* p, std::size_t n, std::align_val_t a) noexcept
{
    (void)n;
    (void)a;
    if (p == nullptr)
        return;

    Memory::Free(p);
}
