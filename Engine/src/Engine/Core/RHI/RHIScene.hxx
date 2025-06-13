#pragma once

#include "Engine/Core/RHI/RHICommandList.hxx"
#include "Engine/Core/RHI/RHIContext.hxx"
#include "Engine/GameFramework/Components/CameraComponent.hxx"
#include "Engine/Math/Transform.hxx"
#include "Engine/Threading/Lock.hxx"

class RMeshComponent;
class RWorld;
class RRHIScene;
class RAsset;
class RRHIMaterial;

enum class ERenderSceneLockType
{
    Read,
    Write,
};

struct FRenderRequestKey
{
    RRHIMaterial* Material = nullptr;
    RAsset* Asset = nullptr;

    bool operator==(const FRenderRequestKey& Other) const = default;
};

namespace std
{

// std::hash specialization for FRenderRequestKey
template <>
struct hash<FRenderRequestKey>
{
    FORCEINLINE std::size_t operator()(const FRenderRequestKey& Key) const
    {
        return std::hash<RAsset*>{}(Key.Asset) ^ std::hash<RRHIMaterial*>{}(Key.Material);
    }
};

}    // namespace std

struct FRHISceneUpdateBatch
{
    FRHISceneUpdateBatch(unsigned Count);

    TArray<FMatrix4, 64> MatrixArray;

    TArray<uint64> Actors;

    TArray<float, 64> PositionX;
    TArray<float, 64> PositionY;
    TArray<float, 64> PositionZ;

    TArray<float, 64> ScaleX;
    TArray<float, 64> ScaleY;
    TArray<float, 64> ScaleZ;

    TArray<float, 64> QuaternionX;
    TArray<float, 64> QuaternionY;
    TArray<float, 64> QuaternionZ;
    TArray<float, 64> QuaternionW;
};

template <ERenderSceneLockType LockType>
class TRenderSceneLock
{
public:
    TRenderSceneLock(WeakRef<RRHIScene> InScene);
    ~TRenderSceneLock();

private:
    WeakRef<RRHIScene> Scene = nullptr;
};

class RRHIScene : public RObject
{
    RTTI_DECLARE_TYPEINFO(RRHIScene, RObject);

public:
    struct FRHIRenderPassTarget
    {
        WeakRef<RRHIViewport> Viewport = nullptr;

        TArray<FRHIRenderTarget> ColorTargets = {};
        std::optional<FRHIRenderTarget> DepthTarget = std::nullopt;
        UVector2 Size = {0, 0};
    };

    BEGIN_PARAMETER_STRUCT(UCameraData)
    PARAMETER(FMatrix4, ViewProjection)
    PARAMETER(FMatrix4, View)
    PARAMETER(FMatrix4, Projection)
    END_PARAMETER_STRUCT();

private:
    struct FMeshRepresentation
    {
        FTransform Transform = {};
        uint32 TransformBufferIndex = std::numeric_limits<uint32>::max();
        uint32 RenderBufferIndex = std::numeric_limits<uint32>::max();
        WeakRef<RMeshComponent> Mesh = nullptr;
    };

    struct FActorRepresentationUpdateRequest
    {
        uint64 ActorId = 0;
        FTransform NewTransform = {};
    };

public:
    RRHIScene() = delete;
    RRHIScene(RWorld* OuterWorld);
    RRHIScene(RWorld* OuterWorld, const FRHIRenderPassTarget& InRenderPassTarget);
    virtual ~RRHIScene();

    void SetViewport(Ref<RRHIViewport>& InViewport);
    RRHIViewport* GetViewport()
    {
        return RenderPassTarget.Viewport.Raw();
    }
    void SetRenderPassTarget(const FRHIRenderPassTarget& InRenderPassTarget);

    void PreTick();
    void PostTick(double DeltaTime);
    void UpdateActorLocation(uint64 Id, const FTransform& NewTransform);

    void TickRenderer(FFRHICommandList& CommandList);

private:
    void UpdateCameraAspectRatio();

    void Async_UpdateActorRepresentations(FRHISceneUpdateBatch& Batch);

private:
    FRHIRenderPassTarget RenderPassTarget;

    UCameraData CameraData;
    Ref<RRHIBuffer> u_CameraBuffer = nullptr;

    TMap<uint64, TResourceArray<FMatrix4>> TransformResourceArray;
    TMap<uint64, Ref<RRHIBuffer>> TransformBuffers;
    TMap<FRenderRequestKey, TArray<FMeshRepresentation*>> RenderCalls;

    TMap<uint64, TArray<FMeshRepresentation>> WorldActorRepresentation;
    TArray<WeakRef<RCameraComponent<float>>> CameraComponents;

    FRWFifoLock ContextLock;
    FRHIContext* const Context = nullptr;

    std::future<void> AsyncTaskUpdateResult;

    std::mutex ActorAttentionMutex;
    TMap<uint64, FActorRepresentationUpdateRequest> ActorThatNeedAttention;

    template <ERenderSceneLockType LockType>
    friend class TRenderSceneLock;
};

template <ERenderSceneLockType LockType>
inline TRenderSceneLock<LockType>::TRenderSceneLock(WeakRef<RRHIScene> InScene): Scene(InScene)
{
    if constexpr (LockType == ERenderSceneLockType::Read)
    {
        RPH_PROFILE_FUNC("TRenderSceneLock - Read Lock");
        Scene->ContextLock.ReadLock();
    }
    else
    {
        RPH_PROFILE_FUNC("TRenderSceneLock - Write Lock");
        Scene->ContextLock.WriteLock();
    }
}

template <ERenderSceneLockType LockType>
inline TRenderSceneLock<LockType>::~TRenderSceneLock()
{
    if constexpr (LockType == ERenderSceneLockType::Read)
    {
        RPH_PROFILE_FUNC("TRenderSceneLock - Read Unlock");
        Scene->ContextLock.ReadUnlock();
    }
    else
    {
        RPH_PROFILE_FUNC("TRenderSceneLock - Write Unlock");
        Scene->ContextLock.WriteUnlock();
    }
}
