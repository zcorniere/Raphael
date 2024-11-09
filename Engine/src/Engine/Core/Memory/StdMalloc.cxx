#include "Engine/Core/Memory/StdMalloc.hxx"

void* FStdMalloc::Alloc(uint32 Size, uint32)
{
    return std::malloc(Size);
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
