#pragma once

#include "Engine/Core/UUID.hxx"

#include "Engine/Renderer/RHI/GenericRHI.hxx"
#include "Engine/Renderer/RHI/RHIResource.hxx"

#include <functional>

class PassNode;

class ResourceEntry : public UUID, public RObject
{
public:
    ResourceEntry(Ref<RHIResource> InResource = nullptr, uint32 InVersion = 1);

    virtual void ConstructResource() = 0;
    virtual void DestroyResource() = 0;

    uint32 GetVersion() const { return m_Version; }

protected:
    uint32 m_Version;
    Ref<RHIResource> m_Resource;

    Ref<PassNode> Producer;
    Ref<PassNode> Last;

    friend class FrameGraph;
};

template <RHIResourceType Type, typename... Args>
class TResourceEntry : public ResourceEntry
{
public:
    TResourceEntry(uint32 InVersion, const Args &...args): ResourceEntry(nullptr, InVersion), Arguments(args...) {}

    void ConstructResource() override
    {
        verify(m_Resource);
        m_Resource = std::apply(GenericRHI::Create<Type, Args...>, Arguments);
    }
    void DestroyResource() override { m_Resource = nullptr; }

private:
    const std::tuple<Args...> Arguments;
};
