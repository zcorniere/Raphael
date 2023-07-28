#pragma once

#include "Engine/Core/Memory/Memory.hxx"

/// Allocator using mimalloc
class AllocatorPoison : public Malloc
{
public:
    constexpr static int AllocFillNew = 0xbb;
    constexpr static int AllocFillFree = 0xcc;

public:
    AllocatorPoison() = delete;
    explicit AllocatorPoison(Malloc* InMalloc): TrueMalloc(InMalloc)
    {
        check(TrueMalloc);
    }

    virtual void* Alloc(uint32 Size, uint32 Alignment = 0) override
    {
        void* Ptr = TrueMalloc->Alloc(Size, Alignment);
        if (LIKELY(Ptr != nullptr && Size > 0)) {
            std::memset(Ptr, AllocFillNew, Size);
        }
        return Ptr;
    }
    virtual void* Realloc(void* Original, uint32 Size, uint32 Alignment = 0) override
    {
        unsigned OldSize = 0;
        if (Original != nullptr && GetAllocationSize(Original, OldSize) && OldSize > 0 && OldSize > Size) {
            std::memset(static_cast<uint8*>(Original) + Size, AllocFillFree, OldSize - Size);
        }

        void* Result = TrueMalloc->Realloc(Original, Size, Alignment);

        if (Result != nullptr && OldSize > 0 && OldSize < Size) {
            std::memset(static_cast<uint8*>(Result) + OldSize, AllocFillNew, Size - OldSize);
        }

        return Result;
    }

    virtual void Free(void* Ptr) override
    {
        if (LIKELY(Ptr)) {
            unsigned AllocSize;
            if (LIKELY(GetAllocationSize(Ptr, AllocSize) && AllocSize > 0)) {
                std::memset(Ptr, AllocFillFree, AllocSize);
            }
            TrueMalloc->Free(Ptr);
        }
    }

    virtual bool GetAllocationSize(void* Ptr, uint32& OutSize) override
    {
        return TrueMalloc->GetAllocationSize(Ptr, OutSize);
    }
    virtual const char* GetAllocatorName() const override
    {
        return TrueMalloc->GetAllocatorName();
    }

private:
    Malloc* TrueMalloc;
};
