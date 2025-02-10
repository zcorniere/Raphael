#include "Engine/Core/RHI/RHIScene.hxx"

#include "Engine/Core/RHI/GenericRHI.hxx"
#include "Engine/Core/RHI/RHI.hxx"

RHIScene::RHIScene(): Context(RHI::Get()->RHIGetCommandContext())
{
    u_CameraBuffer = RHI::CreateBuffer(FRHIBufferDesc{
        .Size = sizeof(UCameraData),
        .Stride = sizeof(UCameraData),
        .Usage = EBufferUsageFlags::UniformBuffer | EBufferUsageFlags::KeepCPUAccessible,    // @TODO: not that
        .ResourceArray = nullptr,
        .DebugName = "Camera Buffer",
    });
    CameraData.View = FMatrix4::Identity();
    CameraData.Projection = FMatrix4::Identity();
    CameraData.ViewProjection = FMatrix4::Identity();
}

RHIScene::~RHIScene()
{
    RHI::Get()->RHIReleaseCommandContext(Context);
}

void RHIScene::CollectRenderablesSystem(ecs::FTransformComponent& Transform, ecs::FMeshComponent& Mesh)
{
    RenderRequests[Mesh.Asset->GetName()].Emplace(FRenderRequest{Transform, Mesh});
}

void RHIScene::CameraSystem(ecs::FCameraComponent& Camera)
{
    // @TODO: support multiple cameras
    if (Camera.bIsActive) {
        Camera.ViewPoint.SetAspectRatio(Viewport->GetAspectRatio());
        CameraData.View = Camera.ViewPoint.GetViewMatrix();
        CameraData.Projection = Camera.ViewPoint.GetProjectionMatrix();
        CameraData.ViewProjection = CameraData.Projection * CameraData.View;
        bCameraDataDirty = true;
    }
}

void RHIScene::Tick(float DeltaTime)
{
    (void)DeltaTime;
    if (bCameraDataDirty) {
        ENQUEUE_RENDER_COMMAND(UpdateCameraBuffer)([this](FFRHICommandList& CommandList) {
            TResourceArray<UCameraData> Array{CameraData};
            CommandList.CopyRessourceArrayToBuffer(&Array, u_CameraBuffer, 0, 0, sizeof(UCameraData));
        });

        bCameraDataDirty = false;
    }
}

void RHIScene::TickRenderer(FFRHICommandList& CommandList)
{
    CommandList.SetViewport(
        {0, 0, 0}, {static_cast<float>(Viewport->GetSize().x), static_cast<float>(Viewport->GetSize().y), 1.0f});
    CommandList.SetScissor({0, 0}, {Viewport->GetSize().x, Viewport->GetSize().y});

    RHIRenderPassDescription Description{
        .RenderAreaLocation = {0, 0},
        .RenderAreaSize = Viewport->GetSize(),
        .ColorTargets =
            {
                RHIRenderTarget{
                    .Texture = Viewport->GetBackbuffer(),
                    .ClearColor = {0.0f, 0.0f, 0.0f, 1.0f},
                    .LoadAction = ERenderTargetLoadAction::Clear,
                    .StoreAction = ERenderTargetStoreAction::Store,
                },
            },
        .DepthTarget = std::nullopt,
    };
    CommandList.BeginRenderingViewport(Viewport.Raw());
    CommandList.BeginRendering(Description);

    for (auto& [AssetName, RenderRequests]: RenderRequests) {
        for (FRenderRequest& Request: RenderRequests) {
            if (!Request.Mesh.Asset->IsLoadedOnGPU()) {
                Request.Mesh.Asset->LoadOnGPU();
                continue;
            }

            Request.Mesh.Material->SetInput("Camera", u_CameraBuffer);
            Request.Mesh.Material->Bind();

            RAsset::FDrawInfo Cube = Request.Mesh.Asset->GetDrawInfo();
            CommandList.SetVertexBuffer(Request.Mesh.Asset->GetVertexBuffer(), 0, 0);
            CommandList.DrawIndexed(Request.Mesh.Asset->GetIndexBuffer(), 0, 0, Cube.NumVertices, 0, Cube.NumPrimitives,
                                    1);
        }
    }
    CommandList.EndRendering();
    CommandList.EndRenderingViewport(Viewport.Raw());

    RenderRequests.clear();
}
