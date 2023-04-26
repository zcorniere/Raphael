#include "Engine/Core/FrameGraph/FrameGraph.hxx"

#include "Engine/Core/RHI/RHIResource.hxx"

#include "Engine/Core/FrameGraph/PassNode.hxx"
#include "Engine/Core/FrameGraph/ResourceEntry.hxx"

FrameGraph::FrameGraph() {}

FrameGraph::~FrameGraph() {}

bool FrameGraph::Compile() { return false; }

void FrameGraph::Execute()
{
    for (auto &resource: m_ResourceRegistry) { resource->ConstructResource(); }
}

bool FrameGraph::IsValid(FrameGraphResource Id) const
{
    const Ref<ResourceNode> &Node = GetResourceNode(Id);
    const Ref<ResourceEntry> &Resource = m_ResourceRegistry[Node->m_ResourceIndex];

    return Node->m_Version == Resource->m_Version;
}

FrameGraphResource FrameGraph::Clone(FrameGraphResource Id)
{
    const Ref<ResourceNode> &Node = GetResourceNode(Id);
    Ref<ResourceEntry> &Entry = GetResourceEntry(Node->m_ResourceIndex);
    Entry->m_Version++;

    Ref<ResourceNode> NewNode =
        Ref<ResourceNode>::CreateNamed(Node->GetName(), Node->m_ResourceIndex, Entry->GetVersion());
    m_ResourceNode.emplace_back(NewNode);
    return NewNode->ID();
}

const Ref<ResourceNode> &FrameGraph::GetResourceNode(FrameGraphResource Id) const { return m_ResourceNode[Id]; }

Ref<ResourceEntry> &FrameGraph::GetResourceEntry(FrameGraphResource Id)
{
    const Ref<ResourceNode> &Node = GetResourceNode(Id);
    return m_ResourceRegistry[Node->m_ResourceIndex];
}
