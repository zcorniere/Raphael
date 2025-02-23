#include "EditorApplication.hxx"

#include "Components/Oscillator.hxx"
#include "Engine/Core/ECS/Component/CameraComponent.hxx"
#include "Engine/Core/ECS/Component/MeshComponent.hxx"
#include "Engine/Core/ECS/Component/RHIComponent.hxx"
#include "Engine/Core/Engine.hxx"
#include "Engine/Math/Vector.hxx"

#include <Engine/Core/ECS/World.hxx>
#include <Engine/Core/Log.hxx>
#include <Engine/Core/RHI/RHICommandList.hxx>
#include <Engine/Core/RHI/Resources/RHIViewport.hxx>
#include <Engine/Platforms/PlatformMisc.hxx>

#include <cpplogger/sinks/FileSink.hpp>
#include <cpplogger/sinks/StdoutSink.hpp>

DECLARE_LOGGER_CATEGORY(Editor, LogApplication, Info)

static cpplogger::Logger* s_EditorLogger = nullptr;

EditorApplication::EditorApplication()
{
    check(s_EditorLogger == nullptr);
    s_EditorLogger = new cpplogger::Logger("Editor");

    s_EditorLogger->addSink<cpplogger::StdoutSink, Log::ColorFormatter>(stdout);
}

EditorApplication::~EditorApplication()
{
    delete s_EditorLogger;
    s_EditorLogger = nullptr;
}

bool EditorApplication::OnEngineInitialization()
{
    RPH_PROFILE_FUNC()

    Super::OnEngineInitialization();

    World = ecs::CreateWorld();
    GEngine->SetWorld(World);
    World->GetScene()->SetViewport(MainViewport);

    World->RegisterComponent<FOscillator>();

    Ref<RAsset> CubeAsset = GEngine->AssetRegistry.GetCubeAsset();
    CubeAsset->LoadOnGPU();
    Ref<RAsset> CapsuleAsset = GEngine->AssetRegistry.GetCapsuleAsset();
    CapsuleAsset->LoadOnGPU();

    FRHIGraphicsPipelineSpecification Spec{
        .VertexShader = "Shapes/ShapeShader.vert",
        .FragmentShader = "Shapes/ShapeShader.frag",
        .VertexBufferLayouts =
            {
                {
                    .InputMode = EVertexInputMode::PerVertex,
                    .Parameter =
                        {
                            {.Name = "Position", .Type = EVertexElementType::Float3},
                            {.Name = "Normal", .Type = EVertexElementType::Float3},
                            {.Name = "Tangent", .Type = EVertexElementType::Float3},
                            {.Name = "Binormal", .Type = EVertexElementType::Float3},
                            {.Name = "Texcoord", .Type = EVertexElementType::Float2},
                        },
                },
                {
                    .InputMode = EVertexInputMode::PerInstance,
                    .Parameter =
                        {
                            {.Name = "MatrixRow_a", .Type = EVertexElementType::Float4},
                            {.Name = "MatrixRow_b", .Type = EVertexElementType::Float4},
                            {.Name = "MatrixRow_c", .Type = EVertexElementType::Float4},
                            {.Name = "MatrixRow_d", .Type = EVertexElementType::Float4},
                        },
                },
            },
        .Rasterizer =
            {
                .PolygonMode = EPolygonMode::Fill,
                .CullMode = ECullMode::None,
                .FrontFaceCulling = EFrontFace::CounterClockwise,
            },
        .AttachmentFormats =
            {
                .ColorFormats = {MainViewport->GetBackbuffer()->GetDescription().Format},
                .DepthFormat = EImageFormat::D32_SFLOAT,
                .StencilFormat = std::nullopt,
            },
    };

    Ref<RRHIGraphicsPipeline> Pipeline = RHI::CreateGraphicsPipeline(Spec);
    Ref<RRHIMaterial> Material = RHI::CreateMaterial(Pipeline);

    FRHITextureSpecification DepthTexture = MainViewport->GetBackbuffer()->GetDescription();
    DepthTexture.Format = EImageFormat::D32_SFLOAT;
    DepthTexture.Flags = ETextureUsageFlags::DepthStencilTargetable;

    ecs::FEntity RenderTarget = World->CreateEntity()
                                    .WithComponent(ecs::FRenderTargetComponent{
                                        .Viewport = MainViewport,
                                    })
                                    .Build();

    for (float x = -5; x < 5; x++) {
        for (float y = -5; y < 5; y++) {
            World->CreateEntity()
                .WithComponent(ecs::FMeshComponent{
                    .Asset = CubeAsset,
                    .Material = Material,
                    .RenderTarget = RenderTarget,
                })
                .WithComponent(FOscillator{
                    .Multiplier = 3.0f,
                    .Direction = {0, 0, 1},
                    .Maximum = {0, 0, 6},
                    .Minimum = {0, 0, -6},
                })
                .WithComponent(
                    ecs::FTransformComponent({x * 2, (y - 10) * 2, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}))
                .Build();
        }
    }

    ecs::FCameraComponent Camera{.bIsActive = true, .ViewPoint = {80.f, 0.01, 1000000.f}};
    Camera.ViewPoint.SetLocation({0.0f, -3.0f, 0.f});

    CameraEntity = World->CreateEntity().WithComponent(Camera).Build();

    World->RegisterSystem([](ecs::FTransformComponent& Transform, FOscillator& Oscillator) {
        if (Transform.GetLocation().z >= Oscillator.Maximum.z || Transform.GetLocation().z <= Oscillator.Minimum.z) {
            Oscillator.Direction.z = -Oscillator.Direction.z;
        }
        if (Transform.GetLocation().y >= Oscillator.Maximum.y || Transform.GetLocation().y <= Oscillator.Minimum.y) {
            Oscillator.Direction.y = -Oscillator.Direction.y;
        }
        if (Transform.GetLocation().x >= Oscillator.Maximum.x || Transform.GetLocation().x <= Oscillator.Minimum.x) {
            Oscillator.Direction.x = -Oscillator.Direction.x;
        }

        FVector3 Delta = Oscillator.Direction * Oscillator.Multiplier * GEngine->GetWorld()->GetDeltaTime();

        FVector3 NewLocation = Transform.GetLocation() + Delta;
        Transform.SetLocation(NewLocation);
    });

    return true;
}

void EditorApplication::OnEngineDestruction()
{
    ecs::DestroyWorld(World);

    Super::OnEngineDestruction();
}

void EditorApplication::Tick(const float DeltaTime)
{
    RPH_PROFILE_FUNC()

    Super::Tick(DeltaTime);

    MainWindow->SetText(std::to_string(1.0f / DeltaTime));
    World->Update(DeltaTime);
}

// Not really extern "C" but I use it to mark that this function will be called by an external unit
extern "C" IApplication* GetApplication()
{
    return new EditorApplication();
}
