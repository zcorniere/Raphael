#include "EditorApplication.hxx"

#include "Engine/Core/ECS/Component/CameraComponent.hxx"
#include "Engine/Core/ECS/Component/MeshComponent.hxx"
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

    World->RegisterComponent<ecs::FTransformComponent>();
    World->RegisterComponent<ecs::FMeshComponent>();
    World->RegisterComponent<ecs::FCameraComponent>();

    Ref<RAsset> CubeAsset = GEngine->AssetRegistry.GetCubeAsset();
    CubeAsset->LoadOnGPU();
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
                .DepthFormat = std::nullopt,
                .StencilFormat = std::nullopt,
            },
    };

    Ref<RMaterial> Material = GEngine->AssetRegistry.LoadMaterial("Default", Spec);
    Entity =
        World->CreateEntity()
            .WithComponent(ecs::FMeshComponent{
                CubeAsset,
                Material,
            })
            .WithComponent(ecs::FTransformComponent({0.0f, -3.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}))
            .Build();
    CameraEntity = World->CreateEntity()
                       .WithComponent(ecs::FCameraComponent({
                           .bIsActive = true,
                           .ViewPoint = {80.f, 0.01, 1000000.f},
                       }))
                       .Build();

    World->RegisterSystem([](ecs::FTransformComponent& Transform) {
        //        Transform.GetRotation().y += 30.f;
        float TransformX = Transform.GetLocation().x + (10 * GEngine->GetWorld()->GetDeltaTime());
        if (TransformX > 6.0f) {
            TransformX = -6.0f;
        }

        FVector3 NewLocation = Transform.GetLocation();
        NewLocation.x = TransformX;
        Transform.SetLocation(NewLocation);
    });
    World->RegisterSystem([](ecs::FCameraComponent& Cam) { Cam.ViewPoint.SetLocation({0.0f, -3.0f, 0.f}); });

    return true;
}

void EditorApplication::OnEngineDestruction()
{
    World->DestroyEntity(Entity);
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
