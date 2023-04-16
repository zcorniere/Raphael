#pragma once

#include "Engine/Core/FrameGraph/FrameGraphPass.hxx"
#include "Engine/Core/FrameGraph/FrameGraphResource.hxx"

class PassNode final : public RObject
{
public:
    PassNode() = delete;
    PassNode(std::string_view InName, Ref<FrameGraphPass> InGraphPass);
    ~PassNode();

    FrameGraphResource MarkRead(FrameGraphResource Id);
    FrameGraphResource MarkWrite(FrameGraphResource Id);

    bool IsCreating(FrameGraphResource Id) const;
    bool IsReading(FrameGraphResource Id) const;
    bool IsWriting(FrameGraphResource Id) const;

    bool CanExecute() const;

private:
    Ref<FrameGraphPass> m_GraphPass;

    std::vector<FrameGraphResource> m_Creates;
    std::vector<FrameGraphResource> m_Reads;
    std::vector<FrameGraphResource> m_Writes;

    friend class FrameGraphBuilder;
};
