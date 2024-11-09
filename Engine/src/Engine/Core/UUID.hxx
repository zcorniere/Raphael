#pragma once

namespace Raphael
{

// "UUID" (universally unique identifier) or GUID is (usually) a 128-bit integer
// used to "uniquely" identify information. We use the term
// GUID and UUID, at the moment we're simply using a randomly generated 64-bit
// integer, as the possibility of a clash is low enough for now.
// This may change in the future.
class FUUID
{
public:
    FUUID();
    explicit FUUID(uint64 uuid);
    FUUID(const FUUID& other);

    uint64 ID() const
    {
        return m_UUID;
    }

    operator uint64()
    {
        return m_UUID;
    }
    operator uint64() const
    {
        return m_UUID;
    }

private:
    uint64 m_UUID;
};

}    // namespace Raphael

namespace std
{

template <>
struct hash<Raphael::FUUID> {
    std::size_t operator()(const Raphael::FUUID& uuid) const
    {
        // uuid is already a randomly generated number, and is suitable as a hash key as-is.
        return uuid;
    }
};

}    // namespace std
