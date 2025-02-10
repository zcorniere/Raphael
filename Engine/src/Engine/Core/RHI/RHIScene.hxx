#pragma once

#include "Engine/Core/RHI/RHICommandList.hxx"
#include "Engine/Core/RHI/RHIContext.hxx"
#include "Engine/Core/RHI/RHIShaderParameters.hxx"

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

    std::unordered_map<std::string, TArray<FRenderRequest>> RenderRequests;

    WeakRef<RRHIViewport> Viewport = nullptr;
    FRHIContext* const Context = nullptr;
};
