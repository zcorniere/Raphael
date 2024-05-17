#pragma once

template <typename T, typename TSizeType>
requires std::unsigned_integral<TSizeType>
class HeapAllocator
{
public:
    using SizeType = TSizeType;

public:
    HeapAllocator() = default;

    HeapAllocator(SizeType NumElements, SizeType ElementSize, SizeType Alignment = 0)
    {
        Resize(NumElements, ElementSize, Alignment);
    }

    HeapAllocator(const HeapAllocator& Other)
    {
        uint32 MemorySize = 0;
        check(Memory::GetAllocationSize(Other.Data, MemorySize));
        const uint32 Size = MemorySize / sizeof(T);

        Resize(Size, sizeof(T));
        CopyItems(Data, Other.Data, Size);
    }

    HeapAllocator(HeapAllocator&& Other)
    {
        MoveFrom(Other);
    }

    ~HeapAllocator()
    {
        if (Data) {
            Memory::Free(Data);
        }
    }

    void MoveFrom(HeapAllocator& Other)
    {
        check(this != &Other);
        if (Data) {
            Memory::Free(Data);
        }
        Data = Other.Data;
        Other.Data = nullptr;
    }

    void Resize(TSizeType NumElements, TSizeType ElementSize, TSizeType Alignment = 0)
    {
        if (Data || NumElements) {
            // check for overflow/underflow
            check(NumElements >= 0);
            check(ElementSize > 1);
            check(NumElements < std::numeric_limits<TSizeType>::max() / ElementSize);

            Data = (T*)Memory::Realloc(Data, NumElements * ElementSize, Alignment);
        }
    }

    bool HasAllocation() const
    {
        return Data != nullptr;
    }

    void* GetAllocation() const
    {
        return Data;
    }

private:
    T* Data = nullptr;
};
