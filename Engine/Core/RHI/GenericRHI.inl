///
/// Generic RHI inline implementation
///

template <typename TRHI>
Ref<TRHI> GenericRHI::Get()
{
    check(GDynamicRHI);
    return GDynamicRHI.As<TRHI>();
}

template <typename TFunction>
void GenericRHI::Submit(TFunction &&func)
{
    RHICommandQueue::RenderCommandFn RenderCmd = [](void *ptr) {
        TFunction *pFunction = (TFunction *)ptr;
        (*pFunction)();

        pFunction->~TFunction();
    };

    void *pStorageBuffer = GenericRHI::Get<GenericRHI>()->GetRHICommandQueue()->Allocate(RenderCmd, sizeof(func));
    new (pStorageBuffer) TFunction(std::forward<TFunction>(func));
}
