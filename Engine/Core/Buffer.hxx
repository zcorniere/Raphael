#pragma once

namespace Raphael
{

template <typename T>
/// Check if the type is safe to use in a GLSL shader
concept PlainOldDataAligned =
    std::is_standard_layout_v<T> && std::is_trivially_copyable_v<T> && std::is_trivially_destructible_v<T> && requires
{
    sizeof(T) % 4 == 0;
};

template <typename T>
/// Check if the type is usable in a GLSL shader and allow the void type
concept BufferValid = PlainOldDataAligned<T> || std::is_same_v<T, void>;

template <BufferValid T>
class Buffer : public RObject
{
public:
    static Buffer Copy(const T *Data, std::uint32_t Size)
    {
        Buffer<T> Buffer;
        Buffer.Allocate(Size, Data);
        return Buffer;
    }

public:
    Buffer(T *Data = nullptr, std::uint32_t Size = 0): p_Data(Data), m_Size(Size)
    {
    }
    ~Buffer()
    {
        Release();
    }

    void Allocate(std::uint32_t Size, T *Data = nullptr)
    {
        Release();

        if (Size == 0) return;

        if constexpr (std::is_same_v<T, void>) {
            p_Data = new std::byte[Size];
        } else {
            p_Data = new T[Size];
        }
        m_Size = Size;

        if (Data) { std::memcpy(p_Data, Data, Size); }
    }

    void Release()
    {
        if (p_Data) delete[] p_Data;
        p_Data = nullptr;
        m_Size = 0;
    }

    template <typename T2>
    T2 &Read(std::uint32_t offset = 0) requires(!std::is_same_v<T, void>)
    {
        return *(T2 *)p_Data + offset;
    }

    void Write(const T *Data, std::uint32_t Size, std::uint32_t Offset = 0) requires(!std::is_same_v<T, void>)
    {
        check(p_Data);
        check(m_Size >= Offset + Size);
        std::memcpy(p_Data + Offset, Data, Size);
    }

    operator bool() const
    {
        return p_Data;
    }

    T &operator[](unsigned Index)
    {
        return p_Data[Index];
    }
    const T &operator[](int Index) const
    {
        return p_Data[Index];
    }

    template <PlainOldDataAligned T2>
    T2 *As() const
    {
        return (T2 *)p_Data;
    }

    inline std::uint32_t GetSize() const
    {
        return m_Size;
    }

private:
    T *p_Data;
    std::uint32_t m_Size;
};

}    // namespace Raphael
