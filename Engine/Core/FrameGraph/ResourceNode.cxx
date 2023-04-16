#include "Engine/Core/FrameGraph/ResourceNode.hxx"

#include "Engine/Core/FrameGraph/PassNode.hxx"

ResourceNode::ResourceNode(uint32 resourceId, uint32 Version): m_ResourceIndex(resourceId), m_Version(Version) {}
