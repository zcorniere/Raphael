#pragma once

#include "Engine/Core/FrameGraph/FrameGraphPass.hxx"
#include "Engine/Core/FrameGraph/FrameGraphResource.hxx"

#include "Engine/Core/RHI/RHIResource.hxx"

class PassNode;
class ResourceNode;
class ResourceEntry;

class FrameGraph
{
    RPH_NONCOPYABLE(FrameGraph)
public:
    FrameGraph();
    ~FrameGraph();

    template <typename Data, typename SetupCallback, typename ExecuteCallback>
        requires CValidFrameGraphSetupLambda<SetupCallback, Data> &&
                 CValidFrameGraphExecutionLambda<ExecuteCallback, Data>
    const Data &AddCallbackPass(const std::string_view Name, SetupCallback &&setup, ExecuteCallback &&Exec);

    bool Compile();
    void Execute();

    bool IsValid(FrameGraphResource Id) const;

    FrameGraphResource Clone(FrameGraphResource Id);

private:
    template <RHIResourceType Type, typename... Args>
    FrameGraphResource CreateResource(const std::string_view, Args... args);

    const Ref<ResourceNode> &GetResourceNode(FrameGraphResource Id) const;
    Ref<ResourceEntry> &GetResourceEntry(FrameGraphResource Id);

private:
    std::vector<Ref<PassNode>> m_PassNodes;
    std::vector<Ref<ResourceNode>> m_ResourceNode;
    std::vector<Ref<ResourceEntry>> m_ResourceRegistry;

    friend class FrameGraphBuilder;
};

#include "FrameGraph.inl"
