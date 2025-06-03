#pragma once

namespace Raphael
{

class FUUID
{
public:
    FUUID();
    explicit FUUID(uint64 uuid);
    FUUID(const FUUID& other);

    FORCEINLINE uint64 ID() const
    {
        return m_UUID;
    }

    FORCEINLINE operator uint64() const
    {
        return m_UUID;
    }

private:
    const uint64 m_UUID;
};

}    // namespace Raphael

FORCEINLINE bool operator==(const Raphael::FUUID& lhs, const Raphael::FUUID& rhs)
{
    return lhs.ID() == rhs.ID();
}

namespace std
{

template <>
struct hash<Raphael::FUUID>
{
    std::size_t operator()(const Raphael::FUUID& uuid) const
    {
        // uuid is already a randomly generated number, and is suitable as a hash key as-is.
        return uuid;
    }
};

}    // namespace std
