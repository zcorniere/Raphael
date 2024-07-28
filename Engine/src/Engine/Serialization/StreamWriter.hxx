#pragma once

class StreamWriter
{
public:
    virtual ~StreamWriter() = default;

    virtual bool IsGood() const = 0;
    virtual uint64_t GetStreamPosition() = 0;
    virtual void SetStreamPosition(uint64_t position) = 0;
    virtual bool WriteData(const uint8* Data, size_t Size) = 0;

    operator bool() const
    {
        return IsGood();
    }

    template <typename T>
    void WriteRaw(const T& Value)
    {
        WriteData(reinterpret_cast<const uint8*>(&Value), sizeof(T));
    }
    template <typename T>
    void WriteObject(const T& Value)
    {
        T::Serialize(this, Value);
    }

    void WriteString(const std::string& String);

    template <typename T>
    void WriteArray(const Array<T>& Array, bool bWriteSize = true)
    {
        if (bWriteSize) {
            WriteRaw<uint32>(static_cast<uint32>(Array.Size()));
        }

        for (const T& Element: Array) {
            if constexpr (std::is_trivial<T>()) {
                WriteRaw(Element);
            } else if constexpr (std::is_same<T, std::string>()) {
                WriteString(Element);
            } else {
                WriteObject(Element);
            }
        }
    }

    template <typename KeyType, typename ValueType>
    void WriteMap(const std::unordered_map<KeyType, ValueType>& Map, bool bWriteSize = true)
    {
        if (bWriteSize) {
            WriteRaw<uint32>(static_cast<uint32>(Map.size()));
        }

        for (const auto& [Key, Value]: Map) {
            if constexpr (std::is_trivial<KeyType>()) {
                WriteRaw(Key);
            } else if constexpr (std::is_same<KeyType, std::string>()) {
                WriteString(Key);
            } else {
                WriteObject(Key);
            }

            if constexpr (std::is_trivial<ValueType>()) {
                WriteRaw(Value);
            } else {
                WriteObject(Value);
            }
        }
    }
};
