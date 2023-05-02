#pragma once

#include "Engine/Core/RHI/RHICommandQueue.hxx"
#include "Engine/Core/RHI/RHIResource.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogGenericRHI, Info);

enum class RHIInterfaceType {
    Null,
    Vulkan,
};

class GenericRHI;
extern Ref<GenericRHI> GDynamicRHI;

class RHI
{
public:
    template <typename TRHI = GenericRHI>
    static Ref<TRHI> Get()
    {
        check(GDynamicRHI);
        return GDynamicRHI.As<TRHI>();
    }

    static Ref<GenericRHI> CreateRHI();

    static void DeleteRHI();

    template <typename TFunction>
    static void Submit(TFunction &&func);

    static Ref<RHICommandQueue> &GetRHICommandQueue();

    /// RHI Operations

    static void BeginFrame();
    static void EndFrame();
    static void NextFrame();

    static Ref<RHIViewport> CreateViewport(void *InWindowHandle, glm::uvec2 InSize);
    static Ref<RHITexture> CreateTexture(const RHITextureCreateDesc InDesc);
    static Ref<RHIShader> CreateShader(const std::filesystem::path Path, bool bForceCompile);

private:
    static void Init();
};

#include "RHI.inl"
