#include "EditorApplication.hxx"

#include <Engine/Core/Log.hxx>
#include <Engine/Core/RHI/RHICommandList.hxx>
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

    ResourceArray<uint32> TestArray;
    TestArray.Append({11, 22, 33, 44, 55});
    Buffer = RHI::CreateBuffer(RHIBufferDesc{
        .Size = 200,
        .Stride = 0,
        .Usage = EBufferUsageFlags::KeepCPUAccessible,
        .ResourceArray = &TestArray,
        .DebugName = "Test",
    });

    Pipeline = RHI::CreateGraphicsPipeline(RHIGraphicsPipelineSpecification{
        .VertexShader = "DefaultTriangle.vert",
        .PixelShader = "DefaultTriangle.frag",
        .Rasterizer =
            {
                .PolygonMode = EPolygonMode::Fill,
                .CullMode = ECullMode::None,
                .FrontFaceCulling = EFrontFace::Clockwise,
            },
        .AttachmentFormats =
            {
                .ColorFormats = {MainViewport->GetBackbuffer()->GetDescription().Format},
                .DepthFormat = std::nullopt,
                .StencilFormat = std::nullopt,
            },
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
    ENQUEUE_RENDER_COMMAND(BeginFrame)([](RHICommandList& CommandList) { CommandList.BeginFrame(); });

    BaseApplication::Tick(DeltaTime);

    MainWindow->SetText(std::to_string(1.0f / DeltaTime));

    ENQUEUE_RENDER_COMMAND(EmptyRender)
    ([this](RHICommandList& CommandList) {
        RHIRenderPassDescription Description{
            .RenderAreaLocation = {0, 0},
            .RenderAreaSize = MainViewport->GetSize(),
            .ColorTargets =
                {
                    RHIRenderTarget{
                        .Texture = MainViewport->GetBackbuffer(),
                        .ClearColor = {0.0f, 0.0f, 0.0f, 1.0f},
                        .LoadAction = ERenderTargetLoadAction::Clear,
                        .StoreAction = ERenderTargetStoreAction::Store,
                    },
                },
            .DepthTarget = std::nullopt,
        };
        CommandList.BeginRenderingViewport(MainViewport.Raw());
        CommandList.BeginRendering(Description);

        CommandList.TmpDraw(Pipeline);

        CommandList.EndRendering();
        CommandList.EndRenderingViewport(MainViewport.Raw(), true);
    });

    ENQUEUE_RENDER_COMMAND(EndFrame)([](RHICommandList& CommandList) { CommandList.EndFrame(); });
}

// Not really extern "C" but I use it to mark that this function will be called by an external unit
extern "C" IApplication* GetApplication()
{
    return new EditorApplication();
}
