#pragma once

#include "Engine/Containers/Array.hxx"
#include "Engine/Misc/MiscDefines.hxx"

#include "Engine/Serialization/StreamReader.hxx"
#include "Engine/Serialization/StreamWriter.hxx"

#include <cstring>

template <CCharacter TChar>
class TStringView;

template <CCharacter TChar>
class TString
{
public:
    static void Serialize(Serialization::StreamWriter* Writer, const TString& Value)
    {
        Writer->WriteRaw<uint32>(Value.Size());
        Writer->WriteRaw<uint8>(sizeof(TChar));
        Writer->WriteData(reinterpret_cast<const uint8*>(Value.Raw()), Value.Size() * sizeof(TChar));
    }

    static void Deserialize(Serialization::StreamReader* Reader, TString& OutValue)
    {
        uint32 Size = 0;
        Reader->ReadRaw<uint32>(Size);

        uint8 CharSize = 0;
        Reader->ReadRaw<uint8>(CharSize);
        check(CharSize == sizeof(TChar));

        OutValue.Resize(Size);
        Reader->ReadData(reinterpret_cast<uint8*>(OutValue.Raw()), Size * CharSize);
    }

public:
    constexpr TString() = default;
    constexpr TString(const TChar* str, unsigned int size): m_Data(str, size + 1)
    {
        m_Data[size] = '\0';
    }
    constexpr TString(const TChar* str): TString<TChar>(str, std::strlen(str))
    {
    }
    constexpr TString(const TChar* Begin, const TChar* End): TString(Begin, End - Begin)
    {
    }
    constexpr TString(const std::basic_string<TChar>& str): TString(str.c_str(), str.size())
    {
    }
    constexpr TString(const TStringView<TChar>& str): TString(str.Raw(), str.Size())
    {
    }

    constexpr void ToLowerCase()
    {
        for (unsigned int i = 0; i < this->Size(); ++i)
            this->Raw()[i] = std::tolower(this->Raw()[i]);
    }
    constexpr TString ToLowerCase() const
    {
        TString Result = *this;
        Result.ToLowerCase();
        return Result;
    }

    constexpr void ToUpperCase()
    {
        for (unsigned int i = 0; i < this->Size(); ++i)
            this->Raw()[i] = std::toupper(this->Raw()[i]);
    }
    constexpr TString ToUpperCase() const
    {
        TString Result = *this;
        Result.ToUpperCase();
        return Result;
    }

    constexpr const TChar* Raw() const
    {
        return m_Data.Raw();
    }
    constexpr TChar* Raw()
    {
        return m_Data.Raw();
    }

    constexpr void Resize(unsigned NewSize)
    {
        m_Data.Resize(NewSize + 1);
        m_Data[NewSize] = 0;
    }

    constexpr void Clear()
    {
        m_Data.Clear();
    }

    constexpr unsigned Size() const
    {
        return m_Data.Size() - 1;
    }

    constexpr bool IsEmpty() const
    {
        return Size() == 0;
    }

    constexpr TChar& operator[](unsigned index)
    {
        check(index < Size());
        return m_Data[index];
    }

    constexpr const TChar& operator[](unsigned index) const
    {
        check(index < Size());
        return m_Data[index];
    }

    constexpr bool operator==(const TString& Other) const
    {
        if (this->Size() != Other.Size())
            return false;
        for (unsigned int i = 0; i < this->Size(); ++i)
            if (this->Raw()[i] != Other.Raw()[i])
                return false;
        return true;
    }

    constexpr TString& operator+=(const TString& Other)
    {
        return operator+=(Other.Raw());
    }
    constexpr TString& operator+=(const TChar* Other)
    {
        unsigned NewSize = std::strlen(Other);
        unsigned OldSize = Size();

        // Add an extra byte for the null terminator if there is not already one
        m_Data.Resize(OldSize + NewSize + (Size() == 0 ? 0 : 1));
        std::memcpy(m_Data.Raw() + OldSize, Other, NewSize);
        m_Data[OldSize + NewSize] = 0;    // replace the null terminator at the end
        return *this;
    }

private:
    Array<TChar> m_Data;
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const TString<T>& m)
{
    os << std::format("{}", m);
    return os;
}

template <typename T>
struct std::formatter<TString<T>> : std::formatter<T> {

    template <class FormatContext>
    auto format(const TString<T>& Value, FormatContext& ctx) const
    {
        auto&& out = ctx.out();
        format_to(out, "{:s}", Value.Raw());
        return out;
    }
};

using String = TString<char>;
