#include "Engine/Core/Memory/MiMalloc.hxx"

#include <mimalloc.h>

void* FMiMalloc::Alloc(uint32 Size, uint32 Alignment)
{
    void* NewPtr = nullptr;

    uint32 AlignSize(Size >= 16 ? 16 : 8);
    if (Alignment != 0)
    {
        Alignment = std::max(AlignSize, Alignment);
        NewPtr = mi_malloc_aligned(Size, Alignment);
    }
    else
    {
        NewPtr = mi_malloc_aligned(Size, AlignSize);
    }
    return NewPtr;
}

void* FMiMalloc::Realloc(void* Original, uint32 Size, uint32 Alignment)
{
    void* NewPtr = nullptr;

    if (Size == 0)
    {
        mi_free(Original);

        return nullptr;
    }

    if (Alignment != 0)
    {
        Alignment = std::max(Size >= 16 ? (uint32)16 : (uint32)8, Alignment);
        NewPtr = mi_realloc_aligned(Original, Size, Alignment);
    }
    else
    {
        NewPtr = mi_realloc(Original, Size);
    }
    return NewPtr;
}

void FMiMalloc::Free(void* Ptr)
{
    if (!Ptr)
    {
        return;
    }
    mi_free(Ptr);
}

bool FMiMalloc::GetAllocationSize(void* Ptr, uint32& OutSize)
{
    OutSize = mi_malloc_size(Ptr);
    return true;
}
