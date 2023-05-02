#include "Engine/Core/FrameGraph/ResourceEntry.hxx"

#include "Engine/Core/FrameGraph/PassNode.hxx"

ResourceEntry::ResourceEntry(Ref<RHIResource> InResource, uint32 InVersion)
    : m_Version(InVersion), m_Resource(InResource)
{
}
