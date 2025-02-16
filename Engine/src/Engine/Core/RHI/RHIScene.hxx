#pragma once

#include "Engine/Core/RHI/RHICommandList.hxx"
#include "Engine/Core/RHI/RHIContext.hxx"

#include "Engine/Core/ECS/Component/CameraComponent.hxx"
#include "Engine/Core/ECS/Component/MeshComponent.hxx"
#include "Engine/Core/ECS/Component/RHIComponent.hxx"

class RHIScene : public RObject
{
    RTTI_DECLARE_TYPEINFO(RHIScene, RObject);

public:
    BEGIN_PARAMETER_STRUCT(UCameraData)
    PARAMETER(FMatrix4, ViewProjection)
    PARAMETER(FMatrix4, View)
    PARAMETER(FMatrix4, Projection)
    END_PARAMETER_STRUCT();

private:
    struct FRenderRequest {
        ecs::FTransformComponent Transform;
        ecs::FMeshComponent Mesh;
    };

public:
    RHIScene();
    virtual ~RHIScene();

    void CollectRenderablesSystem(ecs::FTransformComponent& Transform, ecs::FMeshComponent& Mesh);
    void CameraSystem(ecs::FCameraComponent& Camera);
    void CollectRenderTargets(ecs::FRenderTargetComponent& RenderTarget);

    void SetViewport(Ref<RRHIViewport>& InViewport)
    {
        Viewport = InViewport;
    }

    void Tick(float DeltaTime);

    void TickRenderer(FFRHICommandList& CommandList);

private:
    bool bCameraDataDirty = true;
    UCameraData CameraData;
    Ref<RRHIBuffer> u_CameraBuffer = nullptr;

    TArray<ecs::FRenderTargetComponent> RenderTargets;
    std::unordered_map<std::string, Ref<RRHIBuffer>> TransformBuffers;
    std::unordered_map<std::string, TArray<FRenderRequest>> RenderRequestMap;

    WeakRef<RRHIViewport> Viewport = nullptr;
    FRHIContext* const Context = nullptr;
};
