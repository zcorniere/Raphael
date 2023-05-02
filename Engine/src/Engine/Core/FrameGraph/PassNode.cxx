#include "Engine/Core/FrameGraph/PassNode.hxx"

#include <ranges>

PassNode::PassNode(std::string_view InName, Ref<FrameGraphPass> InGraphPass): m_GraphPass(InGraphPass)
{
    SetName(InName);
}
PassNode::~PassNode() {}

FrameGraphResource PassNode::MarkRead(FrameGraphResource Id)
{
    check(!IsCreating(Id) && !IsWriting(Id));
    return IsReading(Id) ? Id : m_Reads.emplace_back(Id);
}

FrameGraphResource PassNode::MarkWrite(FrameGraphResource Id) { return IsWriting(Id) ? Id : m_Writes.emplace_back(Id); }

bool PassNode::IsCreating(FrameGraphResource Id) const { return std::ranges::find(m_Creates, Id) != m_Creates.cend(); }

bool PassNode::IsReading(FrameGraphResource Id) const { return std::ranges::find(m_Reads, Id) != m_Reads.cend(); }

bool PassNode::IsWriting(FrameGraphResource Id) const { return std::ranges::find(m_Writes, Id) != m_Writes.cend(); }

bool PassNode::CanExecute() const { return GetRefCount() > 0; }
