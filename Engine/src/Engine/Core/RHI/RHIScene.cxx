#include "Engine/Core/RHI/RHIScene.hxx"

#include "Engine/Core/RHI/GenericRHI.hxx"
#include "Engine/Core/RHI/RHI.hxx"

#include "Engine/GameFramework/Actor.hxx"
#include "Engine/GameFramework/CameraActor.hxx"
#include "Engine/GameFramework/Components/MeshComponent.hxx"
#include "Engine/GameFramework/World.hxx"

RRHIScene::RRHIScene(RWorld* OwnerWorld): Context(RHI::Get()->RHIGetCommandContext())
{
    u_CameraBuffer = RHI::CreateBuffer(FRHIBufferDesc{
        .Size = sizeof(UCameraData),
        .Stride = sizeof(UCameraData),
        .Usage = EBufferUsageFlags::UniformBuffer | EBufferUsageFlags::KeepCPUAccessible,
        .ResourceArray = nullptr,
        .DebugName = "Camera Buffer",
    });

    CameraData.View = FMatrix4::Identity();
    CameraData.Projection = FMatrix4::Identity();
    CameraData.ViewProjection = FMatrix4::Identity();

    // Register to the world events
    OwnerWorld->OnActorAddedToWorld.Add(this,
                                        [this](AActor* Actor) mutable
                                        {
                                            TRenderSceneLock<ERenderSceneLockType::Write> Lock(this);
                                            TArray<FMeshRepresentation>& Representation =
                                                WorldActorRepresentation.Emplace(Actor->ID());

                                            RMeshComponent* const MeshComponent = Actor->GetMesh();
                                            if (MeshComponent)
                                            {
                                                FMeshRepresentation& Mesh = Representation.Emplace();
                                                Mesh.Transform = Actor->GetRootComponent()->GetRelativeTransform();
                                                Mesh.Mesh = MeshComponent;

                                                NewlyAddedActors.Add(Actor->ID());
                                            }

                                            RCameraComponent<float>* CameraComponent =
                                                Actor->GetComponent<RCameraComponent<float>>();
                                            if (CameraComponent)
                                            {
                                                CameraComponents.Add(CameraComponent);
                                            }
                                        });

    OwnerWorld->OnActorRemovedFromWorld.Add(
        this,
        [this](AActor* Actor) mutable
        {
            TRenderSceneLock<ERenderSceneLockType::Write> Lock(this);
            TPair<uint64, TArray<FMeshRepresentation>> DeletedMesh;
            WorldActorRepresentation.Remove(Actor->ID(), &DeletedMesh);

            for (FMeshRepresentation& Mesh: DeletedMesh.Get<1>())
            {
                TResourceArray<FMatrix4>* TransformArrays = TransformResourceArray.Find(Mesh.Mesh->Asset->GetName());
                if (TransformArrays)
                {
                    TransformArrays->RemoveAt(Mesh.TransformBufferIndex);
                }

                FRenderRequestKey Key{Mesh.Mesh->Material.Raw(), Mesh.Mesh->Asset.Raw()};
                TArray<FMeshRepresentation*>* RenderRequests = RenderCalls.Find(Key);
                if (RenderRequests)
                {
                    RenderRequests->RemoveAt(Mesh.RenderBufferIndex);
                }
            }
        });
}

RRHIScene::RRHIScene(RWorld* OwnerWorld, const FRHIRenderPassTarget& InRenderPassTarget): RRHIScene(OwnerWorld)
{
    RenderPassTarget = InRenderPassTarget;
}

RRHIScene::~RRHIScene()
{
    RHI::Get()->RHIReleaseCommandContext(Context);
}

void RRHIScene::SetViewport(Ref<RRHIViewport>& InViewport)
{
    RenderPassTarget.Viewport = InViewport;
}

void RRHIScene::SetRenderPassTarget(const FRHIRenderPassTarget& InRenderPassTarget)
{
    RenderPassTarget = InRenderPassTarget;
}

void RRHIScene::PreTick()
{
    {
        RPH_PROFILE_FUNC("RRHIScene::Tick - Take care of the new actors")
        for (uint64 ActorID: NewlyAddedActors)
        {
            TArray<FMeshRepresentation>* Iter = WorldActorRepresentation.Find(ActorID);
            ensure(Iter);

            for (FMeshRepresentation& Mesh: *Iter)
            {
                if (Mesh.Mesh->Asset == nullptr || Mesh.Mesh->Material == nullptr)
                {
                    continue;
                }
                if (!Mesh.Mesh->Material->WasBaked())
                {
                    Mesh.Mesh->Material->SetInput("Camera", u_CameraBuffer);
                    Mesh.Mesh->Material->Bake();
                }
                TResourceArray<FMatrix4>& RequestArrays = TransformResourceArray.FindOrAdd(Mesh.Mesh->Asset->GetName());
                RequestArrays.Add(Mesh.Transform.GetModelMatrix());
                Mesh.TransformBufferIndex = RequestArrays.Size() - 1;

                FRenderRequestKey Key{Mesh.Mesh->Material.Raw(), Mesh.Mesh->Asset.Raw()};
                TArray<FMeshRepresentation*>& RenderRequests = RenderCalls.FindOrAdd(Key);
                RenderRequests.Add(&Mesh);
                Mesh.RenderBufferIndex = RenderRequests.Size() - 1;
            }
        }
        NewlyAddedActors.Clear();
    }
}

void RRHIScene::PostTick(double DeltaTime)
{
    RPH_PROFILE_FUNC()

    (void)DeltaTime;
    ensure(CameraComponents.Size() == 1);
    if (CameraComponents.IsEmpty() || !CameraComponents[0]->IsValid())
    {
        return;
    }

    RCameraComponent<float>* CameraComponent = CameraComponents[0];
    UpdateCameraAspectRatio();
    if (CameraComponent->IsTransformDirty() || CameraComponent->IsRenderStateDirty())
    {
        RPH_PROFILE_FUNC("RRHIScene::Tick - UpdateCameraBuffer")

        CameraData.View = CameraComponent->GetViewMatrix();
        CameraData.Projection = CameraComponent->GetProjectionMatrix();
        CameraData.ViewProjection = CameraData.Projection * CameraData.View;
        CameraComponent->ClearDirtyTransformFlag();
        CameraComponent->ClearRenderStateDirtyFlag();

        ENQUEUE_RENDER_COMMAND(UpdateCameraBuffer)(
            [this](FFRHICommandList& CommandList)
            {
                TResourceArray<UCameraData> Array{CameraData};
                CommandList.CopyResourceArrayToBuffer(&Array, u_CameraBuffer, 0, 0, sizeof(UCameraData));
            });
    }

    {
        RPH_PROFILE_FUNC("RRHIScene::Tick - Update Transform Buffers")
        for (auto& [AssetName, TransformArrays]: TransformResourceArray)
        {
            Ref<RRHIBuffer>& TransformBuffer = TransformBuffers.FindOrAdd(AssetName);
            if (TransformBuffer == nullptr || TransformBuffer->GetSize() < TransformArrays.Size())
            {
                TransformBuffer = RHI::CreateBuffer(FRHIBufferDesc{
                    .Size = static_cast<uint32>(std::max(TransformArrays.Size() * sizeof(FMatrix4),
                                                         sizeof(FMatrix4))),    // @TODO: not that
                    .Stride = sizeof(FMatrix4),
                    .Usage = EBufferUsageFlags::VertexBuffer | EBufferUsageFlags::KeepCPUAccessible,
                    .ResourceArray = &TransformArrays,
                    .DebugName = "Transform Buffer",
                });
            }
            else
            {
                ENQUEUE_RENDER_COMMAND(UpdateTransformBuffer)(
                    [TransformBuffer](FFRHICommandList& CommandList, TResourceArray<FMatrix4> TransformMatrices) mutable
                    {
                        CommandList.CopyResourceArrayToBuffer(&TransformMatrices, TransformBuffer, 0, 0,
                                                              TransformMatrices.GetByteSize());
                    },
                    TransformArrays);
            }
        }
    }
}

void RRHIScene::UpdateActorLocation(uint64 Id, const FTransform& NewTransform)
{
    RPH_PROFILE_FUNC()
    TArray<FMeshRepresentation>* Iter = WorldActorRepresentation.Find(Id);
    ensure(Iter);

    for (FMeshRepresentation& Mesh: *Iter)
    {
        Mesh.Transform = NewTransform;
        if (Mesh.Mesh->Asset == nullptr || Mesh.Mesh->Material == nullptr)
        {
            continue;
        }
        TransformResourceArray.FindOrAdd(Mesh.Mesh->Asset->GetName())[Mesh.TransformBufferIndex] =
            Mesh.Transform.GetModelMatrix();
    }
}

void RRHIScene::TickRenderer(FFRHICommandList& CommandList)
{
    RPH_PROFILE_FUNC()

    // for (const ecs::FRenderTargetComponent& TargetComponent: RenderTargets) {
    UVector2 Size;
    TArray<FRHIRenderTarget> ColorTargets;
    std::optional<FRHIRenderTarget> DepthTarget = std::nullopt;

    if (RenderPassTarget.Viewport)
    {

        ColorTargets = {
            {
                .Texture = RenderPassTarget.Viewport->GetBackbuffer(),
                .ClearColor = {0.0f, 0.0f, 0.0f, 1.0f},
                .LoadAction = ERenderTargetLoadAction::Clear,
                .StoreAction = ERenderTargetStoreAction::Store,
            },
        };
        DepthTarget = {
            .Texture = RenderPassTarget.Viewport->GetDepthBuffer(),
            .ClearColor = {1.0f, 0.0f, 0.0f, 1.0f},
            .LoadAction = ERenderTargetLoadAction::Clear,
            .StoreAction = ERenderTargetStoreAction::Store,
        };
        Size = RenderPassTarget.Viewport->GetSize();

        CommandList.SetViewport({0, 0, 0}, {static_cast<float>(RenderPassTarget.Viewport->GetSize().x),
                                            static_cast<float>(RenderPassTarget.Viewport->GetSize().y), 1.0f});
        CommandList.SetScissor({0, 0},
                               {RenderPassTarget.Viewport->GetSize().x, RenderPassTarget.Viewport->GetSize().y});
        CommandList.BeginRenderingViewport(RenderPassTarget.Viewport.Raw());
    }
    else
    {
        Size = RenderPassTarget.Size;
        ColorTargets = RenderPassTarget.ColorTargets;
        DepthTarget = RenderPassTarget.DepthTarget;
    }

    FRHIRenderPassDescription Description{
        .RenderAreaLocation = {0, 0},
        .RenderAreaSize = Size,
        .ColorTargets = ColorTargets,
        .DepthTarget = DepthTarget,
    };
    CommandList.BeginRendering(Description);

    {
        RPH_PROFILE_FUNC("RRHIScene::TickRenderer - Draw")
        for (auto& [Key, Requests]: RenderCalls)
        {
            if (!Key.Asset->IsLoadedOnGPU())
            {
                Key.Asset->LoadOnGPU();
                continue;
            }
            CommandList.SetMaterial(Key.Material);

            Ref<RRHIBuffer> TransformVertexBuffer = TransformBuffers[Key.Asset->GetName()];
            RAsset::FDrawInfo Cube = Key.Asset->GetDrawInfo();

            ensure(Key.Asset->GetVertexBuffer() != nullptr);
            ensure(TransformVertexBuffer != nullptr);

            CommandList.SetVertexBuffer(Key.Asset->GetVertexBuffer(), 0, 0);
            CommandList.SetVertexBuffer(TransformVertexBuffer, 1, 0);
            CommandList.DrawIndexed(Key.Asset->GetIndexBuffer(), 0, 0, Cube.NumVertices, 0, Cube.NumPrimitives,
                                    Requests.Size());
        }
    }

    CommandList.EndRendering();

    if (RenderPassTarget.Viewport)
    {
        CommandList.EndRenderingViewport(RenderPassTarget.Viewport.Raw());
    }
}

void RRHIScene::UpdateCameraAspectRatio()
{
    ensure(CameraComponents.Size() == 1);
    if (CameraComponents.IsEmpty() || !CameraComponents[0]->IsValid())
    {
        return;
    }
    if (RenderPassTarget.Viewport)
    {
        CameraComponents[0]->SetAspectRatio(RenderPassTarget.Viewport->GetAspectRatio());
    }
    else
    {
        CameraComponents[0]->SetAspectRatio(RenderPassTarget.Size.x / static_cast<float>(RenderPassTarget.Size.y));
    }
}
