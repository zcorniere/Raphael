#include "EditorApplication.hxx"

#include <Engine/Core/Log.hxx>
#include <Engine/Core/RHI/RHICommandList.hxx>
#include <Engine/Core/RHI/RHIShaderParameters.hxx>
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

    ResourceArray<glm::vec2> Vertices({{0.0, -0.5}, {0.5, 0.5}, {-0.5, 0.5}});
    Ref<RHIBuffer> TmpBuffer = RHI::CreateBuffer(RHIBufferDesc{
        .Size = Vertices.GetByteSize(),
        .Stride = 0,
        .Usage = EBufferUsageFlags::VertexBuffer | EBufferUsageFlags::SourceCopy | EBufferUsageFlags::KeepCPUAccessible,
        .ResourceArray = &Vertices,
        .DebugName = "Staging Vertex Buffer",
    });
    ENQUEUE_RENDER_COMMAND(CopyBuffer)
    ([this, TmpBuffer](RHICommandList& CommandList) {
        Buffer = RHI::CreateBuffer(RHIBufferDesc{
            .Size = TmpBuffer->GetSize(),
            .Stride = 0,
            .Usage = EBufferUsageFlags::VertexBuffer | EBufferUsageFlags::DestinationCopy,
            .ResourceArray = nullptr,
            .DebugName = "Vertex Buffer",
        });
        CommandList.CopyBufferToBuffer(TmpBuffer, Buffer, 0, 0, TmpBuffer->GetSize());
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

        CommandList.SetPipeline(Pipeline);
        CommandList.SetVertexBuffer(Buffer);

        CommandList.SetViewport({0, 0, 0}, {MainViewport->GetSize().x, MainViewport->GetSize().y, 1.0f});
        CommandList.SetScissor({0, 0}, {MainViewport->GetSize().x, MainViewport->GetSize().y});

        CommandList.Draw(3, 1, 0);

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
