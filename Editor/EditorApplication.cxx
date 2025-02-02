#include "EditorApplication.hxx"

#include "Engine/Core/ECS/Component/CameraComponent.hxx"
#include "Engine/Core/ECS/Component/MeshComponent.hxx"
#include "Engine/Core/Engine.hxx"
#include "Engine/Math/Vector.hxx"

#include <Engine/Core/ECS/World.hxx>
#include <Engine/Core/Log.hxx>
#include <Engine/Core/RHI/RHICommandList.hxx>
#include <Engine/Core/RHI/RHIShaderParameters.hxx>
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
    World->Viewport = MainViewport;

    World->RegisterComponent<ecs::TTransformComponent<float>>();
    World->RegisterComponent<ecs::FMeshComponent>();
    World->RegisterComponent<ecs::FCameraComponent>();

    Ref<RAsset> CubeAsset = GEngine->AssetRegistry.GetCubeAsset();
    CubeAsset->LoadOnGPU();
    FRHIGraphicsPipelineSpecification Spec{
        .VertexShader = "Shapes/ShapeShader.vert",
        .FragmentShader = "Shapes/ShapeShader.frag",
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
    Entity = World->CreateEntity()
                 .WithComponent(ecs::FMeshComponent{
                     CubeAsset,
                     Material,
                 })
                 .WithComponent(ecs::TTransformComponent<float>{})
                 .Build();
    CameraEntity = World->CreateEntity()
                       .WithComponent(ecs::TCameraComponent<float>({
                           .bIsActive = true,
                           .ViewPoint = {80.f, 0.01, 1000000.f},
                       }))
                       .Build();

    World->RegisterSystem([](ecs::TTransformComponent<float>& Transform) {
        Transform.GetRotation().y += 3.f;
        Transform.SetLocation({0.0f, 3.0f, 0.0f});
    });
    World->RegisterSystem([](ecs::FCameraComponent& Cam) { Cam.ViewPoint.SetLocation({0.0f, -3.0f, 0.f}); });
    World->RegisterSystem(RHI::RenderSystem);
    World->RegisterSystem(RHI::CameraSystem);

    return true;
}

void EditorApplication::OnEngineDestruction()
{
    World->DestroyEntity(Entity);
    World = nullptr;

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
