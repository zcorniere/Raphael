#pragma once

namespace Serialization
{

class FStreamWriter;
class FStreamReader;

template <typename T>
concept IsSerializable = requires(T t, FStreamWriter* writer) {
    { T::Serialize(writer, t) } -> std::same_as<void>;
};

template <typename T>
concept IsDeserializable = requires(T t, FStreamReader* reader) {
    { T::Deserialize(reader, t) } -> std::same_as<void>;
};

template <typename T>
concept IsSerializableType = IsSerializable<T> && IsDeserializable<T>;

}    // namespace Serialization
