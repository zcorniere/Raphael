#pragma once

#include "Engine/Core/Memory/Memory.hxx"

/// Allocator using mimalloc
class FMiMalloc : public IMallocInterface
{
public:
    virtual void* Alloc(uint32 Size, uint32 Alignment = 0) override;
    virtual void* Realloc(void* Original, uint32 Size, uint32 Alignment = 0) override;

    virtual void Free(void* Ptr) override;

    virtual bool GetAllocationSize(void* Ptr, uint32& OutSize) override;
    virtual const char* GetAllocatorName() const override
    {
        return "MiMalloc";
    }

    virtual bool SupportPoison() const override
    {
        return true;
    }
};
