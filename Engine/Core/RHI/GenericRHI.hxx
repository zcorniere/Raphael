#pragma once

#include "Engine/Core/RHI/RHICommandQueue.hxx"
#include "Engine/Core/RHI/RHIResource.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogRHI, Info);

enum class RHIInterfaceType {
    Null,
    Vulkan,
};

class GenericRHI;

extern Ref<GenericRHI> GDynamicRHI;

class GenericRHI : public RObject
{
public:
    template <typename TRHI>
    static Ref<TRHI> Get();

    template <typename TFunction>
    static void Submit(TFunction &&func);

    // ---------------------- RHI Operations --------------------- //
    static void BeginFrame();
    static void EndFrame();
    static void NextFrame();

    template <RHIResourceType Type, typename... Args>
    static Ref<RHIResource> Create(Args... args);

public:
    virtual ~GenericRHI() {}

    // Initialize the RHI
    virtual void Init();

    // Called after the RHI init
    virtual void PostInit();

    // Shutdown the RHI
    virtual void Shutdown();

    virtual RHIInterfaceType GetInterfaceType() const { return RHIInterfaceType::Null; }

protected:
    Ref<RHICommandQueue> &GetRHICommandQueue() { return m_CommandQueue; }

private:
    Ref<RHICommandQueue> m_CommandQueue;
};

#if RAPHAEL_SELECTED_RHI_VULKAN

// Forward declare to avoid circular dep
namespace VulkanRHI
{
class VulkanDynamicRHI;
}

using RHI = VulkanRHI::VulkanDynamicRHI;

#include "RHI/Vulkan/VulkanRHI.hxx"

#else
    #error "Invalid RHI"
#endif

#include "GenericRHI.inl"
