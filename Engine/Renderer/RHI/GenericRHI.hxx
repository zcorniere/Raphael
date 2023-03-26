#pragma once

#include "RHICommandQueue.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogRHI, Info);

enum class RHIInterfaceType {
    Null,
    Vulkan,
};

class GenericRHI;

extern Ref<GenericRHI> GDynamicRHI;

template <typename TRHI>
FORCEINLINE Ref<TRHI> GetDynamicRHI()
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

        void *pStorageBuffer = GetDynamicRHI<GenericRHI>()->GetRHICommandQueue()->Allocate(RenderCmd, sizeof(func));
        new (pStorageBuffer) TFunction(std::forward<TFunction>(func));
    }

public:
    virtual ~GenericRHI()
    {
    }

    // Initialize the RHI
    virtual void Init();

    // Called after the RHI init
    virtual void PostInit() = 0;

    // Shutdown the RHI
    virtual void Shutdown();

    virtual RHIInterfaceType GetInterfaceType() const
    {
        return RHIInterfaceType::Null;
    }

    // ---------------------- RHI Operations --------------------- //
    static void BeginFrame();
    static void EndFrame();
    static void NextFrame();

protected:
    Ref<RHICommandQueue> &GetRHICommandQueue()
    {
        return m_CommandQueue;
    }

private:
    Ref<RHICommandQueue> m_CommandQueue;
};
