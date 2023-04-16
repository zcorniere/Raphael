///
/// Frame Graph Builder implementation
///

#include "Engine/Core/FrameGraph/FrameGraph.hxx"
#include "Engine/Core/FrameGraph/PassNode.hxx"

template <RHIResourceType Type, typename... Args>
FrameGraphResource FrameGraphBuilder::Create(const std::string_view Name, Args &&...args)
{
    FrameGraphResource Id = m_FrameGraph.CreateResource<Type>(Name, std::forward<Args>(args)...);
    return m_PassNode->m_Creates.emplace_back(Id);
}
