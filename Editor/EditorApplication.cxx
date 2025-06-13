#include "EditorApplication.hxx"

#include "Engine/Core/Engine.hxx"
#include "Engine/Core/RHI/RHIScene.hxx"

#include <Engine/Core/Log.hxx>
#include <Engine/Core/RHI/RHICommandList.hxx>
#include <Engine/Core/RHI/Resources/RHIViewport.hxx>
#include <Engine/Platforms/PlatformMisc.hxx>

#include "Actor/Oscillator.hxx"
#include "Engine/GameFramework/CameraActor.hxx"

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
    Material->SetName("Shape Material");
    GEngine->AssetRegistry.RegisterMemoryOnlyMaterial(Material);

    FRHITextureSpecification DepthTexture = MainViewport->GetBackbuffer()->GetDescription();
    DepthTexture.Format = EImageFormat::D32_SFLOAT;
    DepthTexture.Flags = ETextureUsageFlags::DepthStencilTargetable;

    World = GEngine->CreateWorld();
    World->SetName("Editor World");
    GEngine->SetWorld(World);
    World->GetScene()->SetRenderPassTarget({
        MainViewport,
    });

    Ref<ACameraActor> CameraActor =
        World->CreateActor<ACameraActor>("Main Camera", FTransform({0, 15, 0}, {}, {1, 1, 1}));
    RCameraComponent<float>* CameraComponent = CameraActor->GetComponent<RCameraComponent<float>>();
    CameraComponent->SetFOV(80.0f);
    CameraComponent->SetNearFar(0.1f, 1000.0f);

    const unsigned GridSize = 10;
    for (float Row = 0; Row < GridSize; Row++)
    {
        for (float Col = 0; Col < GridSize; Col++)
        {
            float x = (Col - (GridSize - 1) / 2.0f) * 2;
            float y = (Row - (GridSize - 1) / 2.0f) * 2;
            std::string Name = std::format("Oscillator_{:f}_{:f}", x, y);
            FVector3 Position = {x, y, 0};
            FQuaternion Rotation;
            FVector3 Scale = {1, 1, 1};

            World->CreateActor<AOscillator>(Name, FTransform(Position, Rotation, Scale));
        }
    }

    MainViewport->GetSlateInstance(true);

    return true;
}

void EditorApplication::OnEngineDestruction()
{
    World = nullptr;
    GEngine->SetWorld(nullptr);
    Super::OnEngineDestruction();
}

void EditorApplication::Tick(const double DeltaTime)
{
    RPH_PROFILE_FUNC()

    Super::Tick(DeltaTime);

        MainViewport->GetSlateInstance()->Rect(0, 0, 800, 800, {.169, .169, .169, 1});
        MainViewport->GetSlateInstance()->Rect(10, 10, 100, 100, {.169, .552, 0, 1});
    MainWindow->SetText(std::to_string(1.0f / DeltaTime));
}

// Not really extern "C" but I use it to mark that this function will be called by an external unit
extern "C" IApplication* GetApplication()
{
    return new EditorApplication();
}
