#include "EditorApplication.hxx"

#include <Engine/Core/FrameGraph/FrameGraph.hxx>
#include <Engine/Core/Log.hxx>
#include <Engine/Core/RHI/Resources/RHIViewport.hxx>
#include <Engine/Platforms/PlatformMisc.hxx>

#include <cpplogger/sinks/FileSink.hpp>
#include <cpplogger/sinks/StdoutSink.hpp>

DECLARE_LOGGER_CATEGORY(Editor, LogApplication, Warning)

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

    BaseApplication::OnEngineInitialization();

#if 0
    struct FrameData {
        FrameGraphResource Texture;
        FrameGraphResource Shader;
    };
    FrameGraph Graph;
    Graph.AddCallbackPass<FrameData>(
        "Test pass",
        [](FrameGraphBuilder& Builder, FrameData& Data) {
            RHITextureSpecification Description{};
            Data.Texture = Builder.Create<RHIResourceType::Texture>("Test_Texture", Description);
            Data.Shader = Builder.Create<RHIResourceType::Shader>("Triangle_shader",
                                                                  std::filesystem::path("DefaultTriangle.vert"), false);

            LOG(LogApplication, Info, "Setup");
        },
        [](const FrameData&, FrameGraphPassResources&) { LOG(LogApplication, Info, "Execution"); });
    Graph.Compile();
    Graph.Execute();
#endif
    ResourceArray<uint32> TestArray;
    TestArray.Append({11, 22, 33, 44, 55});
    Buffer = RHI::CreateBuffer(RHIBufferDesc{
        .Size = 200,
        .Stride = 0,
        .Usage = EBufferUsageFlags::KeepCPUAccessible,
        .ResourceArray = &TestArray,
        .DebugName = "Test",
    });
    RHIRenderPassDescription Description{
        .ColorTarget =
            {
                {
                    .Format = MainViewport->GetBackbuffer()->GetDescription().Format,
                    .Flags = ETextureUsageFlags::RenderTargetable,
                },
            },
        .DepthTarget = std::nullopt,
    };
    Pipeline = RHI::CreateGraphicsPipeline(RHIGraphicsPipelineSpecification{
        .VertexShader = "DefaultTriangle.vert",
        .PixelShader = "DefaultTriangle.frag",
        .Rasterizer =
            {
                .PolygonMode = EPolygonMode::Fill,
                .CullMode = ECullMode::None,
                .FrontFaceCulling = EFrontFace::Clockwise,
            },
        .RenderPass = Description,
    });
    return true;
}

void EditorApplication::OnEngineDestruction()
{
    Buffer = nullptr;
    Pipeline = nullptr;

    BaseApplication::OnEngineDestruction();
}

void EditorApplication::Tick(const float DeltaTime)
{
    RPH_PROFILE_FUNC()

    BaseApplication::Tick(DeltaTime);

    MainWindow->SetText(std::to_string(1.0f / DeltaTime));

    MainViewport->BeginDrawViewport();

    ENQUEUE_RENDER_COMMAND(MainApplicationTick)
    ([this] {
        RHIRenderPassDescription Description{
            .ColorTarget =
                {
                    {
                        .Format = MainViewport->GetBackbuffer()->GetDescription().Format,
                        .Flags = ETextureUsageFlags::RenderTargetable,
                    },
                },
            .DepthTarget = std::nullopt,
            // .DepthTarget = std::make_optional(RHIRenderPassDescription::RenderingTargetInfo{
            //     .Format = EImageFormat::D32_SFLOAT,
            // }),
        };
        RHIFramebufferDefinition Definition{
            .ColorTarget =
                {
                    MainViewport->GetBackbuffer(),
                },
            .DepthTarget = nullptr,
            .Offset = {0, 0},
            .Extent = MainViewport->GetBackbuffer()->GetDescription().Extent,
        };
        RHI::BeginRenderPass(Description, Definition);
        RHI::Draw(Pipeline);
        RHI::EndRenderPass();
    });

    MainViewport->EndDrawViewport();
}

// Not really extern "C" but I use it to mark that this function will be called by an external unit
extern "C" IApplication* GetApplication()
{
    return new EditorApplication();
}
