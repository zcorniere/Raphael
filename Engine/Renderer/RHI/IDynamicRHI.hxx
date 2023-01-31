#pragma once

namespace Raphael::RHI
{

DECLARE_LOGGER_CATEGORY(Core, LogRHI, Info);

enum class RHIInterfaceType {
    Null,
    Vulkan,
};

class IDynamicRHI : public RObject
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
};

extern Ref<IDynamicRHI> GDynamicRHI;

template <typename TRHI>
FORCEINLINE Ref<TRHI> GetDynamicRHI()
{
    return GDynamicRHI.As<TRHI>();
}

}    // namespace Raphael::RHI
