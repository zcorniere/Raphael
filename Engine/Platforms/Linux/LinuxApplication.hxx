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

    void ProcessEvent(SDL_Event SDLEvent) override;

    void Tick(const float DeltaTime) override;
    bool ShouldExit() const override;

private:
    Ref<Windows::LinuxWindow> FindEventWindow(SDL_Event &Event);

private:
    bool bShouldExit = false;
    Ref<RHI::VulkanDynamicRHI> RHI;

    std::vector<Ref<Windows::LinuxWindow>> Windows;
};

using Application = LinuxApplication;

extern Ref<Application> GApplication;

}    // namespace Raphael
