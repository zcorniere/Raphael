#pragma once

#include "Engine/Misc/MiscDefines.hxx"

template <typename T, unsigned MinimalCapacity, typename SizeType>
class TArray;

template <typename T, typename SizeType = uint32>
class TArrayView
{
public:
    using TSize = SizeType;

public:
    TArrayView() = delete;
    constexpr TArrayView(T* InData, SizeType Size): Data(InData), ArraySize(Size)
    {
    }

    template <unsigned N>
    constexpr TArrayView(T (&Array)[N]): Data(Array)
                                       , ArraySize(N)
    {
    }

    template <unsigned MinimalCapacity>
    constexpr TArrayView(TArray<T, MinimalCapacity, SizeType>& Other): Data(Other.Raw())
                                                                     , ArraySize(Other.Size())
    {
    }

    template <unsigned MinimalCapacity>
    constexpr TArrayView(std::initializer_list<T> List): TArrayView(List.begin(), List.size())
    {
    }

    [[nodiscard]] constexpr T* Raw() const
    {
        return Data;
    }
    [[nodiscard]] constexpr SizeType Size() const
    {
        return ArraySize;
    }
    [[nodiscard]] constexpr T& operator[](SizeType Index)
    {
        checkMsg((Index >= 0) & (Index < TSize(Size())), "Index is out of bounds.");
        return Data[Index];
    }
    [[nodiscard]] constexpr const T& operator[](SizeType Index) const
    {
        checkMsg((Index >= 0) & (Index < TSize(Size())), "Index is out of bounds.");
        return Data[Index];
    }

    [[nodiscard]] constexpr T* begin()
    {
        return Raw();
    }
    [[nodiscard]] constexpr const T* begin() const
    {
        return Raw();
    }

    [[nodiscard]] constexpr T* end()
    {
        return Raw() + Size();
    }
    [[nodiscard]] constexpr const T* end() const
    {
        return Raw() + Size();
    }

    constexpr bool operator==(const TArrayView& Other) const
    {
        if (Other.Raw() == this->Raw())
            return true;
        if (Other.Size() != this->Size())
            return false;

        for (unsigned i = 0; i < Other.Size() && i < this->Size(); i++)
        {
            if ((*this)[i] != Other[i])
            {
                return false;
            }
        }
        return true;
    }

private:
    T* const Data = nullptr;
    SizeType ArraySize = 0;
};
