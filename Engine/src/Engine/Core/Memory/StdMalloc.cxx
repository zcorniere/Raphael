#include "Engine/Core/Memory/StdMalloc.hxx"

void* StdMalloc::Alloc(uint32 Size, uint32)
{
    return std::malloc(Size);
}

void* StdMalloc::Realloc(void* Original, uint32 Size, uint32)
{
    return std::realloc(Original, Size);
}

void StdMalloc::Free(void* Ptr)
{
    return std::free(Ptr);
}

bool StdMalloc::GetAllocationSize(void*, uint32&)
{
    return 0;
}
