#pragma once

#include "Engine/Core/RHI/RHICommandList.hxx"
#include "Engine/Core/RHI/RHIContext.hxx"
#include "Engine/GameFramework/Components/CameraComponent.hxx"
#include "Engine/Math/Transform.hxx"

class RMeshComponent;
class RWorld;

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
        WeakRef<RMeshComponent> Mesh = nullptr;
    };

public:
    RRHIScene() = delete;
    RRHIScene(RWorld* OuterWorld);
    RRHIScene(RWorld* OuterWorld, const FRHIRenderPassTarget& InRenderPassTarget);
    virtual ~RRHIScene();

    void SetViewport(Ref<RRHIViewport>& InViewport);
    void SetRenderPassTarget(const FRHIRenderPassTarget& InRenderPassTarget);

    void Tick(double DeltaTime);
    void UpdateActorLocation(uint64 Id, const FTransform& NewTransform);

    void TickRenderer(FFRHICommandList& CommandList);

private:
    void UpdateCameraAspectRatio();

private:
    FRHIRenderPassTarget RenderPassTarget;

    UCameraData CameraData;
    Ref<RRHIBuffer> u_CameraBuffer = nullptr;

    std::unordered_map<std::string, Ref<RRHIBuffer>> TransformBuffersPerAsset;
    std::unordered_map<uint64, TArray<FMeshRepresentation>> WorldActorRepresentation;
    TArray<WeakRef<RCameraComponent<float>>> CameraComponents;

    FRHIContext* const Context = nullptr;
};
