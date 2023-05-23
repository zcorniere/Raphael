#pragma once

#include "Engine/Core/FrameGraph/FrameGraphResource.hxx"
#include "Engine/Core/RHI/RHIResource.hxx"

class PassNode;
class FrameGraph;

class FrameGraphBuilder
{
    RPH_NONCOPYABLE(FrameGraphBuilder)

    friend class FrameGraph;

public:
    FrameGraphBuilder() = delete;

    template <RHIResourceType Type, typename... Args>
    [[nodiscard]] FrameGraphResource Create(const std::string_view, Args&&... args);

    FrameGraphResource Read(FrameGraphResource Id);
    [[nodiscard]] FrameGraphResource Write(FrameGraphResource Id);

private:
    FrameGraphBuilder(FrameGraph&, Ref<PassNode>);

private:
    FrameGraph& m_FrameGraph;
    Ref<PassNode> m_PassNode;
};

#include "FrameGraphBuilder.inl"
