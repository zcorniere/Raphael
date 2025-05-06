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

enum class ERenderSceneLockType {
    Read,
    Write,
};

struct FRenderRequestKey {
    RRHIMaterial* Material = nullptr;
    RAsset* Asset = nullptr;

    bool operator==(const FRenderRequestKey& Other) const = default;
};

namespace std
{

// std::hash specialization for FRenderRequestKey
template <>
struct hash<FRenderRequestKey> {
    std::size_t operator()(const FRenderRequestKey& Key) const
    {
        return std::hash<RAsset*>{}(Key.Asset) ^ std::hash<RRHIMaterial*>{}(Key.Material);
    }
};

}    // namespace std

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
    struct FRHIRenderPassTarget {
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
    struct FMeshRepresentation {
        FTransform Transform = {};
        uint32 TransformBufferIndex = 0;
        uint32 RenderBufferIndex = 0;
        WeakRef<RMeshComponent> Mesh = nullptr;
    };

public:
    RRHIScene() = delete;
    RRHIScene(RWorld* OuterWorld);
    RRHIScene(RWorld* OuterWorld, const FRHIRenderPassTarget& InRenderPassTarget);
    virtual ~RRHIScene();

    void SetViewport(Ref<RRHIViewport>& InViewport);
    void SetRenderPassTarget(const FRHIRenderPassTarget& InRenderPassTarget);

    void PreTick();
    void PostTick(double DeltaTime);
    void UpdateActorLocation(uint64 Id, const FTransform& NewTransform);

    void TickRenderer(FFRHICommandList& CommandList);

private:
    void UpdateCameraAspectRatio();

private:
    FRHIRenderPassTarget RenderPassTarget;

    UCameraData CameraData;
    Ref<RRHIBuffer> u_CameraBuffer = nullptr;

    TMap<std::string, TResourceArray<FMatrix4>> TransformResourceArray;
    TMap<std::string, Ref<RRHIBuffer>> TransformBuffers;
    TMap<FRenderRequestKey, TArray<FMeshRepresentation*>> RenderCalls;

    TArray<uint64> NewlyAddedActors;
    TMap<uint64, TArray<FMeshRepresentation>> WorldActorRepresentation;
    TArray<WeakRef<RCameraComponent<float>>> CameraComponents;

    FRWFifoLock Lock;
    FRHIContext* const Context = nullptr;

    template <ERenderSceneLockType LockType>
    friend class TRenderSceneLock;
};

template <ERenderSceneLockType LockType>
inline TRenderSceneLock<LockType>::TRenderSceneLock(WeakRef<RRHIScene> InScene): Scene(InScene)
{
    if constexpr (LockType == ERenderSceneLockType::Read) {
        RPH_PROFILE_FUNC("TRenderSceneLock - Read Lock");
        Scene->Lock.ReadLock();
    } else {
        RPH_PROFILE_FUNC("TRenderSceneLock - Write Lock");
        Scene->Lock.WriteLock();
    }
}

template <ERenderSceneLockType LockType>
inline TRenderSceneLock<LockType>::~TRenderSceneLock()
{
    if constexpr (LockType == ERenderSceneLockType::Read) {
        RPH_PROFILE_FUNC("TRenderSceneLock - Read Unlock");
        Scene->Lock.ReadUnlock();
    } else {
        RPH_PROFILE_FUNC("TRenderSceneLock - Write Unlock");
        Scene->Lock.WriteUnlock();
    }
}
