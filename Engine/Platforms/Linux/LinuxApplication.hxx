#pragma once

#include "Engine/Platforms/Application.hxx"
#include "Engine/Platforms/Window.hxx"
#include "Engine/Renderer/Vulkan/VulkanDynamicRHI.hxx"
#include "Engine/Renderer/Vulkan/VulkanResources.hxx"

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
    Ref<LinuxWindow> FindEventWindow(SDL_Event &Event);

private:
    bool bShouldExit = false;
    Ref<VulkanRHI::VulkanDynamicRHI> RHI;

    std::vector<Ref<LinuxWindow>> Windows;
    Ref<VulkanRHI::VulkanViewport> Viewport;
};

using Application = LinuxApplication;

extern Ref<Application> GApplication;
