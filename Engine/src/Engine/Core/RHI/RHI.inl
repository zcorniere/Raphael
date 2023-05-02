/// RHI static implementation

#ifndef RHI_IMPLEMENTATION_FILE
#define RHI_IMPLEMENTATION_FILE

template <typename TFunction>
void RHI::Submit(TFunction &&func)
{
    RHICommandQueue::RenderCommandFn RenderCmd = [](void *ptr) {
        TFunction *pFunction = (TFunction *)ptr;
        (*pFunction)();

        pFunction->~TFunction();
    };

    void *pStorageBuffer = RHI::GetRHICommandQueue()->Allocate(RenderCmd, sizeof(func));
    new (pStorageBuffer) TFunction(std::forward<TFunction>(func));
}

#endif
