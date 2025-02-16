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
    RenderRequestMap[Mesh.Asset->GetName()].Emplace(FRenderRequest{Transform, Mesh});
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

void RHIScene::CollectRenderTargets(ecs::FRenderTargetComponent& RenderTarget)
{
    RenderTargets.AddUnique(RenderTarget);
}

void RHIScene::Tick(float DeltaTime)
{
    RPH_PROFILE_FUNC()

    (void)DeltaTime;
    if (bCameraDataDirty) {
        ENQUEUE_RENDER_COMMAND(UpdateCameraBuffer)([this](FFRHICommandList& CommandList) {
            TResourceArray<UCameraData> Array{CameraData};
            CommandList.CopyRessourceArrayToBuffer(&Array, u_CameraBuffer, 0, 0, sizeof(UCameraData));
        });

        bCameraDataDirty = false;
    }

    check(RenderRequestMap.size() <= 1);    // calm down buddy, where not there yet
    for (auto& [AssetName, RenderRequest]: RenderRequestMap) {

        TResourceArray<FMatrix4> TransformMatrices;
        TransformMatrices.Reserve(RenderRequest.Size());
        for (FRenderRequest& Request: RenderRequest) {
            TransformMatrices.Add(Request.Transform.GetModelMatrix());

            Request.Mesh.Material->SetInput("Camera", u_CameraBuffer);
            Request.Mesh.Material->Bake();
        }

        Ref<RRHIBuffer>& TransformBuffer = TransformBuffers[AssetName];
        if (TransformBuffer == nullptr || TransformBuffer->GetSize() < RenderRequest.Size()) {
            TransformBuffer = RHI::CreateBuffer(FRHIBufferDesc{
                .Size = static_cast<uint32>(std::max(RenderRequest.Size() * sizeof(FMatrix4),
                                                     sizeof(FMatrix4))),    // @TODO: not that
                .Stride = sizeof(FMatrix4),
                .Usage = EBufferUsageFlags::VertexBuffer | EBufferUsageFlags::KeepCPUAccessible,
                .ResourceArray = &TransformMatrices,
                .DebugName = "Transform Buffer",
            });
        } else {
            ENQUEUE_RENDER_COMMAND(UpdateTransformBuffer)(
                [TransformBuffer = TransformBuffer](FFRHICommandList& CommandList,
                                                    TResourceArray<FMatrix4> TransformMatrices) mutable {
                    IResourceArrayInterface* RessourceArray = &TransformMatrices;

                    CommandList.CopyRessourceArrayToBuffer(RessourceArray, TransformBuffer, 0, 0,
                                                           TransformMatrices.GetByteSize());
                },
                TransformMatrices);
        }
    }
}

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

void RHIScene::TickRenderer(FFRHICommandList& CommandList)
{
    RPH_PROFILE_FUNC()

    for (const ecs::FRenderTargetComponent& TargetComponent: RenderTargets) {
        UVector2 Size;
        TArray<FRHIRenderTarget> ColorTargets;
        std::optional<FRHIRenderTarget> DepthTarget = std::nullopt;

        if (TargetComponent.Viewport) {
            ColorTargets = {
                {
                    .Texture = Viewport->GetBackbuffer(),
                    .ClearColor = {0.0f, 0.0f, 0.0f, 1.0f},
                    .LoadAction = ERenderTargetLoadAction::Clear,
                    .StoreAction = ERenderTargetStoreAction::Store,
                },
            };
            DepthTarget = {
                .Texture = Viewport->GetDepthBuffer(),
                .ClearColor = {1.0f, 0.0f, 0.0f, 1.0f},
                .LoadAction = ERenderTargetLoadAction::Clear,
                .StoreAction = ERenderTargetStoreAction::Store,
            };
            Size = Viewport->GetSize();

            CommandList.SetViewport({0, 0, 0}, {static_cast<float>(TargetComponent.Viewport->GetSize().x),
                                                static_cast<float>(TargetComponent.Viewport->GetSize().y), 1.0f});
            CommandList.SetScissor({0, 0},
                                   {TargetComponent.Viewport->GetSize().x, TargetComponent.Viewport->GetSize().y});
            CommandList.BeginRenderingViewport(Viewport.Raw());
        } else {
            Size = TargetComponent.Size;
            ColorTargets = TargetComponent.ColorTargets;
            DepthTarget = TargetComponent.DepthTarget;
        }

        FRHIRenderPassDescription Description{
            .RenderAreaLocation = {0, 0},
            .RenderAreaSize = Size,
            .ColorTargets = ColorTargets,
            .DepthTarget = DepthTarget,
        };
        CommandList.BeginRendering(Description);

        for (auto& [AssetName, RenderRequests]: RenderRequestMap) {
            std::unordered_map<FRenderRequestKey, TArray<FRenderRequest*>> SortedRequests;

            for (FRenderRequest& Request: RenderRequests) {
                if (!Request.Mesh.Asset->IsLoadedOnGPU()) {
                    Request.Mesh.Asset->LoadOnGPU();
                    continue;
                }
                SortedRequests[{Request.Mesh.Material.Raw(), Request.Mesh.Asset.Raw()}].Emplace(&Request);
            }

            for (auto& [Key, Requests]: SortedRequests) {
                CommandList.SetMaterial(Key.Material);

                RAsset::FDrawInfo Cube = Key.Asset->GetDrawInfo();
                CommandList.SetVertexBuffer(Key.Asset->GetVertexBuffer(), 0, 0);
                CommandList.SetVertexBuffer(TransformBuffers[AssetName], 1, 0);
                CommandList.DrawIndexed(Key.Asset->GetIndexBuffer(), 0, 0, Cube.NumVertices, 0, Cube.NumPrimitives,
                                        Requests.Size());
            }
        }
        CommandList.EndRendering();

        if (TargetComponent.Viewport) {
            CommandList.EndRenderingViewport(Viewport.Raw());
        }
    }

    RenderRequestMap.clear();
}
