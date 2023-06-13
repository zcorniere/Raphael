#pragma once

#include <vector>

constexpr static inline auto InvalidVectorIndex = -1;

// This class, for now, only expand std::vector
template <typename T, typename Allocation = Raphael::Allocator<T>, typename TSize = int32>
requires std::is_signed_v<TSize>
class Array : private std::vector<T, Allocation>
{
public:
    Array(): std::vector<T, Allocation>()
    {
    }
    Array(const T* Ptr, const TSize Count): std::vector<T, Allocation>(Ptr, Count)
    {
    }
    Array(const T* Start, const T* End): std::vector<T, Allocation>(Start, End)
    {
    }
    Array(const TSize Count): std::vector<T, Allocation>(Count)
    {
    }
    Array(std::initializer_list<T> InitList): std::vector<T, Allocation>(InitList)
    {
    }

    // Getters
    [[nodiscard]] constexpr auto Size() const -> std::make_unsigned<TSize>::type
    {
        return std::vector<T, Allocation>::size();
    }
    constexpr const T* Raw() const
    {
        return std::vector<T, Allocation>::data();
    }
    constexpr T* Raw()
    {
        return std::vector<T, Allocation>::data();
    }
    [[nodiscard]] constexpr const T& Back() const
    {
        return std::vector<T, Allocation>::back();
    }
    [[nodiscard]] constexpr T& Back()
    {
        return std::vector<T, Allocation>::back();
    }

    // Operators
    [[nodiscard]] FORCEINLINE T& operator[](const TSize Index)
    {
        checkMsg((Index >= 0) & (Index < TSize(Size())), "Index is out of bounds.");
        return std::vector<T, Allocation>::operator[](Index);
    }
    [[nodiscard]] FORCEINLINE const T& operator[](const TSize Index) const
    {
        checkMsg((Index >= 0) & (Index < TSize(Size())), "Index is out of bounds.");
        return std::vector<T, Allocation>::operator[](Index);
    }

    // Helpers
    [[nodiscard]] constexpr bool IsEmpty() const
    {
        return std::vector<T, Allocation>::empty();
    }

    // Operations
    constexpr void Clear()
    {
        return std::vector<T, Allocation>::clear();
    }

    constexpr void Resize(const TSize NewSize)
    {
        return std::vector<T, Allocation>::resize(NewSize);
    }
    constexpr void Reserve(const TSize NewSize)
    {
        return std::vector<T, Allocation>::reserve(NewSize);
    }

    template <typename... ArgsTypes>
    requires std::constructible_from<T, ArgsTypes...>
    constexpr T& Emplace(ArgsTypes&&... args)
    {
        return std::vector<T, Allocation>::emplace_back(std::forward<ArgsTypes>(args)...);
    }
    constexpr T& Add(const T& Value)
    {
        std::vector<T, Allocation>::push_back(Value);
        return Back();
    }
    constexpr T& Add(T&& Value)
    {
        std::vector<T, Allocation>::push_back(std::forward<T>(Value));
        return Back();
    }
    constexpr T Pop()
    {
        T BackType = std::move(std::vector<T, Allocation>::back());
        std::vector<T, Allocation>::pop_back();
        return BackType;
    }

    // Algorithm
    [[nodiscard]] FORCEINLINE bool Find(const T& Item, TSize& Index) const
    {
        Index = this->Find(Item);
        return Index != InvalidVectorIndex;
    }
    [[nodiscard]] TSize Find(const T& Item) const
    {
        for (TSize i = 0; i < Size(); i++) {
            if (Item == (*this)[i]) {
                return i;
            }
        }
        return InvalidVectorIndex;
    }

    void Append(const Array& Source)
    {
        check((void*)this != (void*)&Source);

        for (const T& Item: Source) {
            this->Emplace(Item);
        }
    }

    /// STL For loop compatibility ///
    // I want to return pointer, not some overly complex type, but will do so for now
    constexpr auto begin()
    {
        return std::vector<T, Allocation>::begin();
    }
    constexpr const auto begin() const
    {
        return std::vector<T, Allocation>::begin();
    }

    constexpr auto end()
    {
        return std::vector<T, Allocation>::end();
    }
    constexpr const auto end() const
    {
        return std::vector<T, Allocation>::end();
    }

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
};

template <typename T, typename Allocation, typename TSize>
requires std::is_signed_v<TSize>
std::ostream& operator<<(std::ostream& os, const Array<T, Allocation, TSize>& m)
{
    os << std::formatter<decltype(m)>::format(m);
    return os;
}

template <typename T, typename Allocation, typename TSize>
requires std::is_signed_v<TSize>
struct std::formatter<Array<T, Allocation, TSize>> : std::formatter<T> {
    template <class FormatContext>
    auto format(const Array<T, Allocation, TSize>& Value, FormatContext& ctx) const
    {
        auto&& out = ctx.out();
        format_to(out, "[");
        for (TSize i = 0; i < Value.Size(); i++) {
            format_to(out, "{}{}", Value[i], (i + 1 < Value.Size()) ? ", " : "");
        }
        format_to(out, "]");
        return out;
    }

    std::string value_format;
};
