#pragma once

#include "Engine/Core/Memory/MemoryOperations.hxx"
#include "Engine/Misc/MiscDefines.hxx"

/// Simple array class that uses a custom allocator
template <typename T, unsigned MinimalCapacity = 0, typename SizeType = uint32>
class Array
{
public:
    using TSize = SizeType;
    static const TSize DefaultCapacity = MinimalCapacity;

public:
    /// Initialize the array to the minimal size
    constexpr Array()
    {
        Reserve(MinimalCapacity);
    }
    /// Initialize the array to the given size
    constexpr Array(const TSize Count)
    {
        Resize(Count);
    }
    constexpr Array(const TSize Count, T DefaultValue): Array(Count)
    {
        for (TSize i = 0; i < Count; i++) {
            (*this)[i] = DefaultValue;
        }
    }

    /// Initialize the array by copying the given array
    /// The copy will be done according to the type stored so copy constructors will be called if needed
    constexpr Array(const T* const Ptr, const TSize Count): Array(Count)
    {
        if (Count > 0) {
            CopyItems(Data, Ptr, ArraySize);
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
        Data = Other.Data;
        ArraySize = Other.ArraySize;
        ArrayCapacity = Other.ArrayCapacity;

        Other.Data = nullptr;
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

        Clear(Other.Size());

        if (Other.Size() == 0) {
            return *this;
        }
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

        Data = Other.Data;
        ArraySize = Other.ArraySize;
        ArrayCapacity = Other.ArrayCapacity;

        Other.Data = nullptr;
        Other.ArraySize = 0;
        Other.ArrayCapacity = 0;
        return *this;
    }

    constexpr Array& operator=(std::initializer_list<T> InitList)
    {
        // Resize the array to fit the list, destroy everything, and copy the list into the array
        Resize(InitList.size());
        DestructItems(Raw(), InitList.size());
        CopyItems(Raw(), InitList.begin(), Size());
        return *this;
    }

    /// Get the size of the array
    [[nodiscard]] constexpr TSize Size() const
    {
        return ArraySize;
    }

    /// Get the capacity of the array
    [[nodiscard]] constexpr TSize Capacity() const
    {
        return ArrayCapacity;
    }

    /// Get the size of the array (in byte)
    ///
    /// @code return Size() * sizeof(T); @endcode
    [[nodiscard]] constexpr TSize ByteSize() const
    {
        return Size() * sizeof(T);
    }
    /// Return the raw data pointer
    [[nodiscard]] constexpr const T* Raw() const
    {
        return Data;
    }
    /// Return the raw data pointer
    [[nodiscard]] constexpr T* Raw()
    {
        return Data;
    }

    /// Return the last element of the array
    [[nodiscard]] constexpr const T& Back() const
    {
        const TSize LastIndex = ArraySize > 0 ? ArraySize - 1 : 0;
        return Data[LastIndex];
    }
    /// Return the last element of the array
    [[nodiscard]] constexpr T& Back()
    {
        const TSize LastIndex = ArraySize > 0 ? ArraySize - 1 : 0;
        return Data[LastIndex];
    }

    /// Array access operator
    /// The validity of the index is checked
    [[nodiscard]] constexpr T& operator[](const TSize Index)
    {
        checkMsg((Index >= 0) & (Index < TSize(Size())), "Index is out of bounds.");
        return Data[Index];
    }
    /// Array operators
    /// The validity of the index is checked
    [[nodiscard]] constexpr const T& operator[](const TSize Index) const
    {
        checkMsg((Index >= 0) & (Index < TSize(Size())), "Index is out of bounds.");
        return Data[Index];
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
        if (ExpectedCapacity > 0) {
            Reserve(ExpectedCapacity);
        }
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
        std::optional<TSize> OptIndex = Find(Value);
        if (!OptIndex.has_value()) {
            return false;
        }

        // Move element after Index to the left
        TSize Index = OptIndex.value();
        MoveItems(Raw() + Index, Raw() + Index + 1, Size() - Index - 1);
        ArraySize--;
        return true;
    }

    /// Remove the given element from the array
    constexpr bool RemoveAt(const TSize Index)
    {
        if (Index >= Size())
            return false;

        // Move element after Index to the left
        MoveItems(Raw() + Index, Raw() + Index + 1, Size() - Index - 1);
        ArraySize--;
        return true;
    }

    /// Resize the array to the given size
    constexpr void Resize(const TSize NewSize)
    {
        if (NewSize < ArraySize) {
            DestructItems(Raw() + NewSize, ArraySize - NewSize);
        }
        if (NewSize > ArrayCapacity) {
            Reserve(GetAllocationIncrease(NewSize));
        }
        if (NewSize > ArraySize) {
            ConstructItems(Raw() + ArraySize, NewSize - ArraySize);
        }
        ArraySize = NewSize;
    }
    /// Reserve the given capacity for the array
    constexpr void Reserve(const TSize NewCapacity)
    {
        if (NewCapacity == ArrayCapacity) {
            return;
        }
        if (NewCapacity < ArraySize) {
            Resize(NewCapacity);
        }

        // New capacity is 0, free the data, and return
        // The object should be empty after the call to Resize above
        if (NewCapacity == 0) {
            if (Data) {
                Memory::Free(Data);
                Data = nullptr;
            }
            ArrayCapacity = 0;
            return;
        }

        // Malloc new array, and move the old data to it, then free the old data
        T* const NewData = (T*)Memory::Malloc(NewCapacity * sizeof(T));
        if (Data) {
            MoveItems(NewData, Data, ArraySize);
            Memory::Free(Data);
        }
        Data = NewData;

        ArrayCapacity = NewCapacity;
    }

    /// Construct an element at the end of the array in place
    /// @return The reference to the added element
    template <typename... ArgsTypes>
    constexpr T& Emplace(ArgsTypes&&... args)
    requires std::constructible_from<T, ArgsTypes...>
    {
        IncreaseCapacityIfNeeded(1);

        const TSize LastIndex = ArraySize > 0 ? ArraySize : 0;
        T* const Ptr = Data + LastIndex;
        new (Ptr) T(std::forward<ArgsTypes>(args)...);
        ArraySize++;
        return *Ptr;
    }

    /// Add an element to the end of the array by copying it
    /// @return The reference to the added element
    constexpr T& Add(T& Value)
    {
        IncreaseCapacityIfNeeded(1);

        const TSize LastIndex = ArraySize > 0 ? ArraySize : 0;
        T* const Ptr = Data + LastIndex;
        CopyItems(Ptr, &Value, 1);
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
        IncreaseCapacityIfNeeded(1);

        // We can advance the pointer because the line above ensures that there is enough space
        const TSize LastIndex = ArraySize > 0 ? ArraySize : 0;
        T* const Ptr = Data + LastIndex;
        new (Ptr) T(std::move(Value));
        ArraySize++;
        return *Ptr;
    }

    /// Add an element to the array if it's not already in it
    constexpr bool AddUnique(T&& Value)
    {
        if (!Find(Value).has_value()) {
            Add(std::forward<T>(Value));
            return true;
        }
        return false;
    }
    /// Add an element to the array if if it's not already in it
    constexpr bool AddUnique(T& Value)
    {
        if (!Contains(Value)) {
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
        return Index != std::nullopt;
    }
    /// Find the index of the given element
    /// @note Support char* strings with strcmp
    ///
    /// @arg Item The element to find
    /// @return The index of the element if found, std::nullopt otherwise
    [[nodiscard]] std::optional<TSize> Find(const T& Item) const
    {
        for (TSize i = 0; i < Size(); i++) {
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
        return std::nullopt;
    }

    /// Check if the given index is valid within the array
    [[nodiscard]] FORCEINLINE bool IsValidIndex(const TSize Index) const
    {
        return Index < Size();
    }

    /// Check if the array contains the given element
    /// @return true if the element is in the array
    [[nodiscard]] FORCEINLINE bool Contains(const T& Object) const
    {
        return Find(Object).has_value();
    }

    /// Append the given array to this one
    void Append(const Array& Source)
    {
        check((void*)this != (void*)&Source);

        TSize NewCombinedSize = Size() + Source.Size();
        IncreaseCapacityIfNeeded(NewCombinedSize - Size());

        for (const T& Item: Source) {
            this->Add(Item);
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
    TSize GetAllocationIncrease(TSize NewMinimalCapacity = 0) const
    {
        TSize NewCapacity = ArrayCapacity;
        do {
            NewCapacity = NewCapacity + (NewCapacity % 3) + 1;
        } while (NewCapacity < NewMinimalCapacity);
        return NewCapacity;
    }

    void IncreaseCapacityIfNeeded(TSize Increase)
    {
        if (ArraySize + Increase > ArrayCapacity) {
            TSize NewCapacity = GetAllocationIncrease();
            Reserve(NewCapacity);
        }
    }

private:
    TSize ArraySize = 0;
    TSize ArrayCapacity = 0;

    T* Data = nullptr;
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const Array<T>& m)
{
    os << std::formatter<decltype(m)>::format(m);
    return os;
}

template <typename T>
struct std::formatter<Array<T>> : std::formatter<T> {

    template <class FormatContext>
    auto format(const Array<T>& Value, FormatContext& ctx) const
    {
        auto&& out = ctx.out();
        format_to(out, "[");
        for (typename Array<T>::TSize i = 0; i < Value.Size(); i++) {
            format_to(out, "{}{}", Value[i], (i + 1 < Value.Size()) ? ", " : "");
        }
        format_to(out, "]");
        return out;
    }

    std::string value_format;
};
