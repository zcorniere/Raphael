#pragma once

#include "Engine/Core/UUID.hxx"

class PassNode;

class ResourceNode : public RObject, public UUID
{
public:
    ResourceNode(uint32 resourceId, uint32 Version);

private:
    const uint32 m_ResourceIndex;
    const uint32 m_Version;

    Ref<PassNode> m_Producer;
    Ref<PassNode> m_Last;

    friend class FrameGraph;
};
