#pragma once

#include "Engine/Renderer/RHI/RHICommandQueue.hxx"
#include "Engine/Renderer/RHI/RHIResource.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogRHI, Info);

enum class RHIInterfaceType {
    Null,
    Vulkan,
};

class GenericRHI;

extern Ref<GenericRHI> GDynamicRHI;

template <typename TRHI>
FORCEINLINE Ref<TRHI> GetRHI()
{
    check(GDynamicRHI);
    return GDynamicRHI.As<TRHI>();
}

class GenericRHI : public RObject
{
public:
    template <typename TFunction>
    static void Submit(TFunction &&func)
    {
        RHICommandQueue::RenderCommandFn RenderCmd = [](void *ptr) {
            TFunction *pFunction = (TFunction *)ptr;
            (*pFunction)();

            pFunction->~TFunction();
        };

        void *pStorageBuffer = GetRHI<GenericRHI>()->GetRHICommandQueue()->Allocate(RenderCmd, sizeof(func));
        new (pStorageBuffer) TFunction(std::forward<TFunction>(func));
    }

    // ---------------------- RHI Operations --------------------- //
    static void BeginFrame();
    static void EndFrame();
    static void NextFrame();

    template <RHIResourceType Type, typename... Args>
    static Ref<RHIResource> Create(Args... args);

public:
    virtual ~GenericRHI()
    {
    }

    // Initialize the RHI
    virtual void Init();

    // Called after the RHI init
    virtual void PostInit();

    // Shutdown the RHI
    virtual void Shutdown();

    virtual RHIInterfaceType GetInterfaceType() const
    {
        return RHIInterfaceType::Null;
    }

protected:
    Ref<RHICommandQueue> &GetRHICommandQueue()
    {
        return m_CommandQueue;
    }

private:
    Ref<RHICommandQueue> m_CommandQueue;
};
