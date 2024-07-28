#pragma once

class StreamReader
{
public:
    virtual ~StreamReader() = default;

    virtual bool IsGood() const = 0;
    virtual uint64_t GetStreamPosition() = 0;
    virtual void SetStreamPosition(uint64_t position) = 0;
    virtual bool ReadData(uint8* Data, size_t Size) = 0;

    operator bool() const
    {
        return IsGood();
    }

    template <typename T>
    void ReadRaw(T& Value)
    {
        ReadData(reinterpret_cast<uint8*>(&Value), sizeof(T));
    }

    template <typename T>
    void ReadObject(T& Value)
    {
        T::Deserialize(this, Value);
    }

    void ReadString(std::string& String);

    template <typename T>
    void ReadArray(Array<T>& Array, bool bReadSize = true)
    {
        if (bReadSize) {
            uint32 Size = 0;
            ReadRaw<uint32>(Size);

            Array.Resize(Size);
        }

        for (T& Element: Array) {
            if constexpr (std::is_trivial<T>()) {
                ReadRaw(Element);
            } else if constexpr (std::is_same<T, std::string>()) {
                ReadString(Element);
            } else {
                ReadObject(Element);
            }
        }
    }

    template <typename KeyType, typename ValueType>
    void ReadMap(std::unordered_map<KeyType, ValueType>& Map, bool bReadSize = true)
    {
        if (bReadSize) {
            uint32 Size = 0;
            ReadRaw<uint32>(Size);
            Map.reserve(Size);
        }

        for (uint32 I = 0; I < Map.capacity(); I++) {
            KeyType Key;
            if constexpr (std::is_trivial<KeyType>()) {
                ReadRaw(Key);
            } else if constexpr (std::is_same<KeyType, std::string>()) {
                ReadString(Key);
            } else {
                ReadObject(Key);
            }

            if constexpr (std::is_trivial<ValueType>()) {
                ReadRaw(Map[Key]);
            } else {
                ReadObject(Map[Key]);
            }
        }
    }
};
