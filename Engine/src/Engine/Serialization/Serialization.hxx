#pragma once

namespace Serialization
{

class StreamWriter;
class StreamReader;

template <typename T>
concept IsSerializable = requires(T t, StreamWriter* writer) {
    {
        T::Serialize(writer, t)
    } -> std::same_as<void>;
};

template <typename T>
concept IsDeserializable = requires(T t, StreamReader* reader) {
    {
        T::Deserialize(reader, t)
    } -> std::same_as<void>;
};

template <typename T>
concept IsSerializableType = IsSerializable<T> && IsDeserializable<T>;

}    // namespace Serialization
