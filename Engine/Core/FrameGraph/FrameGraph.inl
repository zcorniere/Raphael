///
/// FrameGraph Implementation file
///

#include "Engine/Core/FrameGraph/FrameGraphBuilder.hxx"
#include "Engine/Core/FrameGraph/PassNode.hxx"
#include "Engine/Core/FrameGraph/ResourceEntry.hxx"
#include "Engine/Core/FrameGraph/ResourceNode.hxx"

#include <functional>
#include <utility>

template <typename Data, typename SetupCallback, typename ExecuteCallback>
    requires ValidFrameGraphSetupLambda<SetupCallback, Data> && ValidFrameGraphExecutionLambda<ExecuteCallback, Data>
const Data &FrameGraph::AddCallbackPass(const std::string_view Name, SetupCallback &&setup, ExecuteCallback &&Exec)

{
    Ref<TFrameGraphPass<Data, ExecuteCallback>> GraphPass =
        Ref<TFrameGraphPass<Data, ExecuteCallback>>::Create(std::forward<ExecuteCallback>(Exec));
    Ref<PassNode> GraphPassNode = Ref<PassNode>::Create(Name, GraphPass);
    m_PassNodes.push_back(GraphPassNode);

    FrameGraphBuilder Builder(*this, GraphPassNode);
    std::invoke(setup, Builder, GraphPass->m_Data);
    return GraphPass->m_Data;
}

template <RHIResourceType Type, typename... Args>
FrameGraphResource FrameGraph::CreateResource(const std::string_view Name, Args... args)
{
    Ref<TResourceEntry<Type>> Entry = Ref<TResourceEntry<Type, Args...>>::CreateNamed(Name, 1u, args...);
    m_ResourceRegistry.push_back(Entry);

    Ref<ResourceNode> Node = Ref<ResourceNode>::CreateNamed(Name, m_ResourceRegistry.size(), 1);
    return m_ResourceNode.emplace_back(Node)->ID();
}
