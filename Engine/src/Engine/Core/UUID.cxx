#include "UUID.hxx"

#include <random>

static std::random_device s_RandomDevice;
static std::mt19937_64 eng(s_RandomDevice());
static std::uniform_int_distribution<uint64> s_UniformDistribution;

namespace Raphael
{

FUUID::FUUID(): m_UUID(s_UniformDistribution(eng))
{
}

FUUID::FUUID(uint64 uuid): m_UUID(uuid)
{
}

FUUID::FUUID(const FUUID& other): m_UUID(other.m_UUID)
{
}

}    // namespace Raphael
