#include "EditorApplication.hxx"

#include "Engine/Core/RHI/Resources/RHIViewport.hxx"
#include <Engine/Core/FrameGraph/FrameGraph.hxx>
#include <Engine/Core/Log.hxx>

#include <cpplogger/sinks/StdoutSink.hpp>

DECLARE_LOGGER_CATEGORY(Editor, LogApplication, Warning)

static cpplogger::Logger* s_EditorLogger = nullptr;

EditorApplication::EditorApplication()
{
    check(s_EditorLogger == nullptr);
    s_EditorLogger = new cpplogger::Logger("Editor");

    s_EditorLogger->addSink<cpplogger::StdoutSink, Log::Formatter>(stdout);
}

EditorApplication::~EditorApplication()
{
    delete s_EditorLogger;
    s_EditorLogger = nullptr;
}

bool EditorApplication::OnEngineInitialization()
{
    RPH_PROFILE_FUNC()

    BaseApplication::OnEngineInitialization();

#if 0
    struct FrameData {
        FrameGraphResource Texture;
        FrameGraphResource Shader;
    };
    FrameGraph Graph;
    Graph.AddCallbackPass<FrameData>(
        "Test pass",
        [](FrameGraphBuilder &Builder, FrameData &Data) {
            RHITextureCreateDesc Description{};
            Data.Texture = Builder.Create<RHIResourceType::Texture>("Test Texture", Description);
            Data.Shader = Builder.Create<RHIResourceType::Shader>("Triangle shader",
                                                                  std::filesystem::path("DefaultTriangle.vert"), false);

            LOG(LogApplication, Info, "Setup");
        },
        [](const FrameData &, FrameGraphPassResources &) { LOG(LogApplication, Info, "Execution"); });
    Graph.Compile();
    Graph.Execute();
#endif

    RHI::CreateGraphicsPipeline(RHIGraphicsPipelineInitializer{
        .VertexShader = "DefaultTriangle.vert",
        .PixelShader = "DefaultTriangle.frag",
        .Rasterizer =
            {
                .PolygonMode = EPolygonMode::Fill,
                .CullMode = ECullMode::Back,
                .FrontFaceCulling = EFrontFace::Clockwise,
            },
        .RenderPass =
            {
                .ColorTarget =
                    {
                        {
                            .Format = EImageFormat::R8G8B8A8_SRGB,
                        },
                    },
                .DepthTarget = std::make_optional(RHIRenderPassDescription::RenderingTargetInfo{
                    .Format = EImageFormat::D32_SFLOAT,
                }),
                .Size = MainViewport->GetSize(),
                .Name = "Simple path",
            },
    });
    return true;
}

void EditorApplication::OnEngineDestruction()
{
    BaseApplication::OnEngineDestruction();
}

void EditorApplication::Tick(const float DeltaTime)
{
    RPH_PROFILE_FUNC()

    BaseApplication::Tick(DeltaTime);

    MainWindow->SetText(std::to_string(1.0f / DeltaTime));

    MainViewport->BeginDrawViewport();

    RHIRenderPassDescription Description{
        .ColorTarget =
            {
                {
                    .Format = EImageFormat::R8G8B8A8_SRGB,
                },
            },
        .DepthTarget = std::make_optional(RHIRenderPassDescription::RenderingTargetInfo{
            .Format = EImageFormat::D32_SFLOAT,
        }),
        .Size = MainViewport->GetSize(),
        .Name = "Simple path",
    };
    RHI::BeginRenderPass(Description);
    RHI::EndRenderPass();

    MainViewport->EndDrawViewport();
}
