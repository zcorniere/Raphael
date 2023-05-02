#include "Engine/Core/FrameGraph/FrameGraphBuilder.hxx"

#include "Engine/Core/FrameGraph/FrameGraph.hxx"
#include "Engine/Core/FrameGraph/PassNode.hxx"

FrameGraphBuilder::FrameGraphBuilder(FrameGraph &InFrameGraph, Ref<PassNode> InPassNode)
    : m_FrameGraph(InFrameGraph), m_PassNode(InPassNode)
{
}

FrameGraphResource FrameGraphBuilder::Read(FrameGraphResource Id)
{
    check(m_FrameGraph.IsValid(Id));
    return m_PassNode->MarkRead(Id);
}

FrameGraphResource FrameGraphBuilder::Write(FrameGraphResource Id)
{
    check(m_FrameGraph.IsValid(Id));

    if (m_PassNode->IsCreating(Id)) {
        return m_PassNode->MarkWrite(Id);
    } else {
        m_PassNode->MarkRead(Id);
        return m_PassNode->MarkWrite(m_FrameGraph.Clone(Id));
    }
}
