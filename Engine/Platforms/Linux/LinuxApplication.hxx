#pragma once

#include "Engine/Platforms/Application.hxx"
#include "Engine/Platforms/Window.hxx"
#include "Engine/Renderer/Vulkan/VulkanRHI.hxx"
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
    Ref<Window> FindEventWindow(SDL_Event &Event);

private:
    bool bShouldExit = false;
    Ref<VulkanRHI::VulkanDynamicRHI> RHI;

    std::vector<Ref<Window>> Windows;
    Ref<VulkanRHI::VulkanViewport> Viewport;
};

using Application = LinuxApplication;

// NOTE: can use Ref<Application> because GenericApplication is handling the Startup/Shutdown sequence of the RObject
// system. Will change when implementing a Engine class or something like that...
extern Application *GApplication;
