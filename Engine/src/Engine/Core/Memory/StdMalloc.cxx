#include "Engine/Core/Memory/StdMalloc.hxx"

// Windows does not have std::aligned_alloc because it is crap
// So windows use a custom allocator by default
#if !defined(PLATFORM_WINDOWS)
void* FStdMalloc::Alloc(uint32 Size, uint32 Alignment)
{
    if (Alignment != 0)
    {
        // Realign the size
        const std::size_t SizeAligned = Size + (Alignment - 1) & ~(Alignment - 1);
        return std::aligned_alloc(Alignment, SizeAligned);
    }
    else
    {
        return std::malloc(Size);
    }
}

void* FStdMalloc::Realloc(void* Original, uint32 Size, uint32)
{
    return std::realloc(Original, Size);
}

void FStdMalloc::Free(void* Ptr)
{
    return std::free(Ptr);
}

bool FStdMalloc::GetAllocationSize(void*, uint32&)
{
    return 0;
}
#endif //PLATFORM_WINDOWS
