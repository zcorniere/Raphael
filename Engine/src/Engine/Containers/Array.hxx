#pragma once

#include "Engine/Core/Memory/Allocator/HeapAllocator.hxx"

#include "Engine/Core/Memory/MemoryOperations.hxx"
#include "Engine/Misc/MiscDefines.hxx"

constexpr static inline auto InvalidVectorIndex = -1;

/// Simple array class that uses a custom allocator
template <typename T, typename AllocationType = HeapAllocator<T, uint32>, AllocationType::SizeType MinimalSize = 10>
class Array
{
public:
    using TSize = AllocationType::SizeType;

public:
    /// Initialize the array to the minimal size
    constexpr Array()
    {
        Reserve(MinimalSize);
    }
    /// Initialize the array to the given size
    constexpr Array(const TSize Count)
    {
        Resize(Count);
    }
    /// Initialize the array by copying the given array
    /// The copy will be done according to the type stored so copy constructors will be called if needed
    constexpr Array(const T* const Ptr, const TSize Count): Array(Count)
    {
        if (Count > 0) {
            CopyItems((T*)Allocator.GetAllocation(), Ptr, ArraySize);
        }
    }
    /// Initialize the array by copying the memory between the two pointers
    /// The copy will be done according to the type stored so copy constructors will be called if needed
    constexpr Array(const T* const Start, const T* const End): Array(Start, End - Start)
    {
    }

    /// Initialize the array by copying the initializer list
    constexpr Array(std::initializer_list<T> InitList): Array(InitList.begin(), InitList.end())
    {
    }

    constexpr Array(const Array& Other): Array(Other.Raw(), Other.Size())
    {
    }

    constexpr Array(Array&& Other) noexcept
    {
        Allocator.MoveFrom(Other.Allocator);
        ArraySize = Other.ArraySize;
        ArrayCapacity = Other.ArrayCapacity;
        Other.ArraySize = 0;
        Other.ArrayCapacity = 0;
    }

    constexpr ~Array()
    {
        Clear();
    }

    constexpr Array& operator=(const Array& Other)
    {
        if (this == &Other) {
            return *this;
        }

        Clear();
        Resize(Other.Size());
        CopyItems(Raw(), Other.Raw(), Size());
        return *this;
    }

    constexpr Array& operator=(Array&& Other) noexcept
    {
        if (this == &Other) {
            return *this;
        }

        Clear();
        Allocator.MoveFrom(Other.Allocator);
        ArraySize = Other.ArraySize;
        ArrayCapacity = Other.ArrayCapacity;
        Other.ArraySize = 0;
        Other.ArrayCapacity = 0;
        return *this;
    }

    /// Get the size of the array
    [[nodiscard]] constexpr auto Size() const -> typename std::make_unsigned<TSize>::type
    {
        return ArraySize;
    }
    /// Get the byte size of the array
    ///
    /// @code return Size() * sizeof(T); @endcode
    [[nodiscard]] constexpr auto ByteSize() const -> typename std::make_unsigned<TSize>::type
    {
        return Size() * sizeof(T);
    }
    /// Return the raw data pointer
    [[nodiscard]] constexpr const T* Raw() const
    {
        return (const T*)Allocator.GetAllocation();
    }
    /// Return the raw data pointer
    [[nodiscard]] constexpr T* Raw()
    {
        return (T*)Allocator.GetAllocation();
    }

    /// Return the last element of the array
    [[nodiscard]] constexpr const T& Back() const
    {
        const TSize LastIndex = ArraySize > 0 ? ArraySize - 1 : 0;
        return ((T*)Allocator.GetAllocation())[LastIndex];
    }
    /// Return the last element of the array
    [[nodiscard]] constexpr T& Back()
    {
        const TSize LastIndex = ArraySize > 0 ? ArraySize - 1 : 0;
        return ((T*)Allocator.GetAllocation())[LastIndex];
    }

    /// Array access operator
    /// The validity of the index is checked
    [[nodiscard]] constexpr T& operator[](const TSize Index)
    {
        checkMsg((Index >= 0) & (Index < TSize(Size())), "Index is out of bounds.");
        return ((T*)Allocator.GetAllocation())[Index];
    }
    /// Array operators
    /// The validity of the index is checked
    [[nodiscard]] constexpr const T& operator[](const TSize Index) const
    {
        checkMsg((Index >= 0) & (Index < TSize(Size())), "Index is out of bounds.");
        return ((T*)Allocator.GetAllocation())[Index];
    }

    /// Return true if the array is empty
    [[nodiscard]] constexpr bool IsEmpty() const
    {
        return Size() == 0;
    }

    /// Empty the array and reserve the given capacity
    constexpr void Clear(TSize ExpectedCapacity = 0)
    {
        Resize(0);
        Reserve(ExpectedCapacity);
    }

    /// Call the given function on each element and clear the array
    template <typename Function>
    void Clear(Function&& Func, TSize ExpectedCapacity = 0)
    requires std::invocable<Function, T&>
    {
        for (TSize i = 0; i < TSize(Size()); i++) {
            Func((*this)[i]);
        }
        return this->Clear(ExpectedCapacity);
    }

    /// Call the delete operator on each element and clear the array
    void Clear(bool DeletePointer, TSize ExpectedCapacity = 0)
    requires std::is_pointer_v<T>
    {
        /// Delete pointer is not used, but it's here to make the function signature different
        (void)DeletePointer;
        return this->Clear([](T Item) { delete Item; }, ExpectedCapacity);
    }

    /// Remove the given element from the array
    constexpr bool Remove(const T& Value)
    {
        TSize Index = Find(Value);
        if (Index == InvalidVectorIndex)
            return false;

        // Move element after Index to the left
        MoveItems(Raw() + Index, Raw() + Index + 1, Size() - Index - 1);
        ArraySize--;
        return true;
    }

    /// Resize the array to the given size
    constexpr void Resize(const TSize NewSize)
    {
        Reserve(NewSize);
        ArraySize = NewSize;
    }
    /// Reserve the given capacity for the array
    constexpr void Reserve(const TSize NewCapacity)
    {
        if (NewCapacity == ArrayCapacity) {
            return;
        }
        if (NewCapacity < ArraySize) {
            DestructItems(Raw() + NewCapacity, ArraySize);
            ArraySize = NewCapacity;
        }

        Allocator.Resize(NewCapacity, sizeof(T));
        if (NewCapacity > ArraySize) {
            ConstructItems(Raw() + ArraySize, NewCapacity - ArraySize);
        }
        ArrayCapacity = NewCapacity;
    }

    /// Construct an element at the end of the array in place
    /// @return The reference to the added element
    template <typename... ArgsTypes>
    constexpr T& Emplace(ArgsTypes&&... args)
    requires std::constructible_from<T, ArgsTypes...>
    {
        SeeIfNeedToIncreaseCapacity(1);

        const TSize LastIndex = ArraySize > 0 ? ArraySize : 0;
        T* const Ptr = ((T*)Allocator.GetAllocation()) + LastIndex;
        new (Ptr) T(std::forward<ArgsTypes>(args)...);
        ArraySize++;
        return *Ptr;
    }

    /// Add an element to the end of the array by copying it
    /// @return The reference to the added element
    constexpr T& Add(T& Value)
    {
        SeeIfNeedToIncreaseCapacity(1);

        const TSize LastIndex = ArraySize > 0 ? ArraySize : 0;
        T* const Ptr = ((T*)Allocator.GetAllocation()) + LastIndex;
        MoveItems(Ptr, &Value, 1);
        T& Item = Back();
        ArraySize++;
        return Item;
    }

    /// Add an element to the end of the array by copying it
    /// @return The reference to the added element
    constexpr T& Add(const T& Value)
    {
        return Emplace(Value);
    }

    /// Add an element to the end of the array by moving it
    /// @return The reference to the added element
    constexpr T& Add(T&& Value)
    {
        SeeIfNeedToIncreaseCapacity(1);

        // We can advance the pointer because the line above ensures that there is enough space
        const TSize LastIndex = ArraySize > 0 ? ArraySize : 0;
        T* const Ptr = ((T*)Allocator.GetAllocation()) + LastIndex;
        new (Ptr) T(std::move(Value));
        ArraySize++;
        return *Ptr;
    }

    /// Add an element to the array if it's not already in it
    constexpr bool AddUnique(T&& Value)
    {
        if (Find(Value) == InvalidVectorIndex) {
            Add(std::forward<T>(Value));
            return true;
        }
        return false;
    }
    /// Add an element to the array if if it's not already in it
    constexpr bool AddUnique(T& Value)
    {
        if (Find(Value) == InvalidVectorIndex) {
            Add(Value);
            return true;
        }
        return false;
    }
    /// Pop the last element of the array
    constexpr T Pop()
    {
        T BackType = std::move(Back());
        DestructItems(&Back(), 1);
        ArraySize--;
        return BackType;
    }

    /// Find the index of the given element
    /// @arg Index The index of the element if found
    /// @return true if the index was found
    [[nodiscard]] FORCEINLINE bool Find(const T& Item, TSize& Index) const
    {
        Index = this->Find(Item);
        return Index != InvalidVectorIndex;
    }
    /// Find the index of the given element
    /// @note Support char* strings with strcmp
    ///
    /// @arg Item The element to find
    /// @return The index of the element if found, InvalidVectorIndex otherwise
    [[nodiscard]] TSize Find(const T& Item) const
    {
        for (TSize i = 0; i < TSize(Size()); i++) {
            if constexpr (std::is_same_v<std::remove_cv<T>, char*>) {
                if (strcmp(Item, (*this)[i]) == 0) {
                    return i;
                }
            } else {
                if (Item == (*this)[i]) {
                    return i;
                }
            }
        }
        return InvalidVectorIndex;
    }

    /// Check if the array contains the given element
    /// @return true if the element is in the array
    [[nodiscard]] FORCEINLINE bool Contains(const T& Object) const
    {
        return Find(Object) != InvalidVectorIndex;
    }

    /// Append the given array to this one
    void Append(const Array& Source)
    {
        check((void*)this != (void*)&Source);

        for (const T& Item: Source) {
            this->Emplace(Item);
        }
    }

    // STL For loop compatibility
    constexpr T* begin()
    {
        return Raw();
    }
    constexpr const T* begin() const
    {
        return Raw();
    }

    constexpr T* end()
    {
        return Raw() + Size();
    }
    constexpr const T* end() const
    {
        return Raw() + Size();
    }

    /// Equality operator
    constexpr bool operator==(const Array& other) const
    {
        if (other.Size() != this->Size())
            return false;

        for (unsigned i = 0; i < other.Size() && i < this->Size(); i++) {
            if ((*this)[i] != other[i]) {
                return false;
            }
        }
        return true;
    }

private:
    TSize GetAllocationIncrease() const
    {
        return ArrayCapacity + (ArrayCapacity % 3) + 1;
    }

    void SeeIfNeedToIncreaseCapacity(TSize Increase)
    {
        if (ArraySize + Increase > ArrayCapacity) {
            ArrayCapacity = GetAllocationIncrease();
            Allocator.Resize(ArrayCapacity, sizeof(T));
        }
    }

private:
    TSize ArraySize = 0;
    TSize ArrayCapacity = 0;
    AllocationType Allocator;
};

template <typename T, typename Allocation>
std::ostream& operator<<(std::ostream& os, const Array<T, Allocation>& m)
{
    os << std::formatter<decltype(m)>::format(m);
    return os;
}

template <typename T, typename Allocation>
struct std::formatter<Array<T, Allocation>> : std::formatter<T> {
    template <class FormatContext>
    auto format(const Array<T, Allocation>& Value, FormatContext& ctx) const
    {
        auto&& out = ctx.out();
        format_to(out, "[");
        for (typename Allocation::TSize i = 0; i < Value.Size(); i++) {
            format_to(out, "{}{}", Value[i], (i + 1 < Value.Size()) ? ", " : "");
        }
        format_to(out, "]");
        return out;
    }

    std::string value_format;
};
