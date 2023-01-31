#pragma once

namespace Raphael
{

template <typename T>
/// Check if the type is safe to use in a GLSL shader
concept PlainOldDataAligned = std::is_standard_layout_v<T> && std::is_trivially_copyable_v<T> &&
                              std::is_trivially_destructible_v<T> && requires { sizeof(T) % 4 == 0; };

template <PlainOldDataAligned T>
class TBuffer : public RObject
{
public:
    static TBuffer Copy(const TBuffer &Other)
    {
        TBuffer buffer;
        buffer.Allocate(Other.Size);
        std::memcpy(buffer.p_Data, Other.p_Data, buffer.Size);
    }

public:
    TBuffer(): p_Data(nullptr), Size(0)
    {
    }
    ~TBuffer()
    {
        Release();
    }

    void Allocate(uint64 InSize)
    {
        delete[] p_Data;

        if (InSize == 0) return;

        p_Data = new T[InSize];
        Size = InSize;
    }
    void Release()
    {
        delete[] p_Data;
        p_Data = nullptr;
        Size = 0;
    }

    T &operator[](int Index)
    {
        return ((T *)p_Data)[Index];
    }
    const T &operator[](int Index) const
    {
        return ((T *)p_Data)[Index];
    }

    void Allocate(uint64 size, const T &DefaultValue)
    {
        Allocate(size);
        if (size == 0) return;

        std::memset(p_Data, DefaultValue, size);
    }

    operator bool() const
    {
        return p_Data;
    }

    const uint64 &GetSize() const
    {
        return Size;
    }

private:
    T *p_Data;
    uint64 Size;
};

using Buffer = TBuffer<uint8>;

}    // namespace Raphael
