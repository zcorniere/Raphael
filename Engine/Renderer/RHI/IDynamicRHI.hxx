#pragma once

namespace Raphael::RHI
{

DECLARE_LOGGER_CATEGORY(Core, LogRHI, Info);

enum class RHIInterfaceType {
    Null,
    Vulkan,
};

class IDynamicRHI
{
public:
    virtual ~IDynamicRHI()
    {
    }

    // Initialize the RHI
    virtual void Init() = 0;

    // Called after the RHI init and before the Render Thread is started
    virtual void PostInit() = 0;

    // Shutdown the RHI
    virtual void Shutdown() = 0;

    virtual const char *GetName() = 0;

    virtual RHIInterfaceType GetInterfaceType() const
    {
        return RHIInterfaceType::Null;
    }
    virtual IDynamicRHI *GetNonValidationRHI()
    {
        return this;
    }
};

extern IDynamicRHI *GDynamicRHI;

template <typename TRHI>
FORCEINLINE TRHI *CastDynamicRHI(IDynamicRHI *InDynamicRHI)
{
    return static_cast<TRHI *>(InDynamicRHI->GetNonValidationRHI());
}

template <typename TRHI>
FORCEINLINE TRHI *GetDynamicRHI()
{
    return CastDynamicRHI<TRHI>(GDynamicRHI);
}

}    // namespace Raphael::RHI
