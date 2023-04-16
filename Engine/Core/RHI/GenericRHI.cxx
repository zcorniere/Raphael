#include "Engine/Core/RHI/GenericRHI.hxx"

Ref<GenericRHI> GDynamicRHI = nullptr;

void GenericRHI::Init()
{
    m_CommandQueue = Ref<RHICommandQueue>::Create();

    GDynamicRHI = this;
}

void GenericRHI::PostInit() {}

void GenericRHI::Shutdown()
{
    GDynamicRHI = nullptr;

    m_CommandQueue = nullptr;
}

// RHI Operation default function
void GenericRHI::BeginFrame() { checkNoEntry(); }
void GenericRHI::EndFrame() { checkNoEntry(); }
void GenericRHI::NextFrame() { checkNoEntry(); }
