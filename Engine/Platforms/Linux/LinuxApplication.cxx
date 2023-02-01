#include "Engine/Platforms/Linux/LinuxApplication.hxx"

namespace Raphael
{

LinuxApplication::LinuxApplication()
{
    RHI = Ref<RHI::VulkanDynamicRHI>::Create();
    Window = Ref<Windows::LinuxWindow>::Create();
}

LinuxApplication::~LinuxApplication()
{
}

bool LinuxApplication::Initialize()
{
    auto WindowDef = std::make_shared<WindowDefinition>();
    WindowDef->Title = "LOL";
    WindowDef->AppearsInTaskbar = true;
    Window->Initialize(WindowDef, nullptr);
    Window->Show();

    RHI->Init();
    return true;
}

void LinuxApplication::Shutdown()
{
    Window->Destroy();

    RHI->Shutdown();
}

void LinuxApplication::Tick(const float DeltaTime)
{
}

bool LinuxApplication::ShouldExit() const
{
    return false;
}

}    // namespace Raphael
