#pragma once

#include <cstdint>
#include <unordered_map>

// "UUID" (universally unique identifier) or GUID is (usually) a 128-bit integer
// used to "uniquely" identify information. We use the term
// GUID and UUID, at the moment we're simply using a randomly generated 64-bit
// integer, as the possibility of a clash is low enough for now.
// This may change in the future.
class UUID
{
public:
    UUID();
    UUID(uint64_t uuid);
    UUID(const UUID &other);
    virtual ~UUID() {}

    uint64_t ID() const { return m_UUID; }

    operator uint64_t() { return m_UUID; }
    operator uint64_t() const { return m_UUID; }

private:
    uint64_t m_UUID;
};

namespace std
{

template <>
struct hash<UUID> {
    std::size_t operator()(const UUID &uuid) const
    {
        // uuid is already a randomly generated number, and is suitable as a hash key as-is.
        // this may change in future, in which case return hash<uint64_t>{}(uuid); might be more appropriate
        return uuid;
    }
};

}    // namespace std
