#pragma once

#include "Engine/Serialization/Serialization.hxx"

namespace Serialization
{

class FStreamReader
{
public:
    virtual ~FStreamReader() = default;

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

    template <IsDeserializable T>
    void ReadObject(T& Value)
    {
        T::Deserialize(this, Value);
    }

    template <IsDeserializable T>
    void ReadObject(std::optional<T>& OptionalValue)
    {
        uint8 Data;
        ReadData(&Data, sizeof(uint8));

        if (Data) {
            T Value;
            ReadObject(Value);
            OptionalValue = Value;
        }
    }

    void ReadString(std::string& String);

    template <typename T>
    void ReadArray(TArray<T>& Array, bool bReadSize = true)
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
    void ReadMap(TMap<KeyType, ValueType>& Map, bool bReadSize = true)
    {
        if (bReadSize) {
            uint32 Size = 0;
            ReadRaw<uint32>(Size);
            Map.Rehash(Size);
        }

        for (uint32 I = 0; I < Map.BucketCount(); I++) {
            KeyType Key;
            if constexpr (std::is_trivial<KeyType>()) {
                ReadRaw(Key);
            } else if constexpr (std::is_same<KeyType, std::string>()) {
                ReadString(Key);
            } else {
                ReadObject(Key);
            }

            ValueType& Value = Map.FindOrAdd(Key);
            if constexpr (std::is_trivial<ValueType>()) {
                ReadRaw(Value);
            } else {
                ReadObject(Value);
            }
        }
    }
};

}    // namespace Serialization
