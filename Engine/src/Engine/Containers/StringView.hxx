#pragma once

#include "Engine/Misc/MiscDefines.hxx"

#include <cstring>

template <CCharacter TChar>
class TString;

template <CCharacter TChar>
class TStringView
{
public:
    constexpr TStringView() = default;
    constexpr TStringView(const TChar* str): TStringView<TChar>(str, std::strlen(str))
    {
    }
    constexpr TStringView(const std::basic_string<TChar>& str): TStringView(str.c_str(), str.size())
    {
    }
    constexpr TStringView(const TString<TChar>& str): TStringView(str.Raw(), str.Size())
    {
    }
    constexpr TStringView(const TChar* str, unsigned int size): pData(str), m_Size(size)
    {
    }

    constexpr unsigned int Size() const
    {
        return m_Size;
    }

    constexpr const TChar* Raw() const
    {
        return pData;
    }
    constexpr const TChar* Raw()
    {
        return pData;
    }

private:
    const TChar* const pData;
    unsigned int m_Size;
};

using StringView = TStringView<char>;
