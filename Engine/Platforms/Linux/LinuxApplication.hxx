#pragma once

#include "Engine/Platforms/Application.hxx"
#include "Engine/Platforms/Window.hxx"
#include "Engine/Renderer/Vulkan/VulkanDynamicRHI.hxx"

namespace Raphael
{

class LinuxApplication : public GenericApplication
{
public:
    LinuxApplication();
    virtual ~LinuxApplication();

    bool Initialize() override;
    void Shutdown() override;

    void Tick(const float DeltaTime) override;
    bool ShouldExit() const override;

private:
    Ref<RHI::VulkanDynamicRHI> RHI;
    Ref<Windows::LinuxWindow> Window;
};

using Application = LinuxApplication;

}    // namespace Raphael
