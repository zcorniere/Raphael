#include "EditorApplication.hxx"

#include "Engine/Math/Matrix.hxx"
#include "Engine/Math/Quaternion.hxx"
#include "Engine/Math/Transform.hxx"
#include "Engine/Math/Vector.hxx"
#include "Engine/Math/ViewPoint.hxx"

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

    Cube = Shapes::CreateBox({1.0f, 1.0f, 1.0f});
}

EditorApplication::~EditorApplication()
{
    delete s_EditorLogger;
    s_EditorLogger = nullptr;
}

bool EditorApplication::OnEngineInitialization()
{
    RPH_PROFILE_FUNC()

    FBaseApplication::OnEngineInitialization();

    Ref<RRHIBuffer> TmpBuffer = RHI::CreateBuffer(FRHIBufferDesc{
        .Size = Cube.Vertices.GetByteSize(),
        .Stride = sizeof(Shapes::FVertex),
        .Usage = EBufferUsageFlags::VertexBuffer | EBufferUsageFlags::SourceCopy | EBufferUsageFlags::KeepCPUAccessible,
        .ResourceArray = &Cube.Vertices,
        .DebugName = "Staging Vertex Buffer",
    });
    Ref<RRHIBuffer> TmpIndexBuffer = RHI::CreateBuffer(FRHIBufferDesc{
        .Size = Cube.Indices.GetByteSize(),
        .Stride = sizeof(uint32),
        .Usage = EBufferUsageFlags::IndexBuffer | EBufferUsageFlags::SourceCopy | EBufferUsageFlags::KeepCPUAccessible,
        .ResourceArray = &Cube.Indices,
        .DebugName = "Staging Index Buffer",
    });

    ENQUEUE_RENDER_COMMAND(CopyBuffer)
    ([this, TmpBuffer, TmpIndexBuffer](FFRHICommandList& CommandList) {
        VertexBuffer = RHI::CreateBuffer(FRHIBufferDesc{
            .Size = TmpBuffer->GetSize(),
            .Stride = sizeof(Shapes::FVertex),
            .Usage = EBufferUsageFlags::VertexBuffer | EBufferUsageFlags::DestinationCopy,
            .ResourceArray = nullptr,
            .DebugName = "Vertex Buffer",
        });
        IndexBuffer = RHI::CreateBuffer(FRHIBufferDesc{
            .Size = TmpIndexBuffer->GetSize(),
            .Stride = sizeof(uint32),
            .Usage = EBufferUsageFlags::IndexBuffer | EBufferUsageFlags::DestinationCopy,
            .ResourceArray = nullptr,
            .DebugName = "Index Buffer",
        });
        CommandList.CopyBufferToBuffer(TmpBuffer, VertexBuffer, 0, 0, TmpBuffer->GetSize());
        CommandList.CopyBufferToBuffer(TmpIndexBuffer, IndexBuffer, 0, 0, TmpIndexBuffer->GetSize());
    });
    Pipeline = RHI::CreateGraphicsPipeline(FRHIGraphicsPipelineSpecification{
        .VertexShader = "Shapes/ShapeShader.vert",
        .FragmentShader = "Shapes/ShapeShader.frag",
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
    VertexBuffer = nullptr;
    IndexBuffer = nullptr;
    StorageBuffer = nullptr;
    Pipeline = nullptr;

    FBaseApplication::OnEngineDestruction();
}

void EditorApplication::Tick(const float DeltaTime)
{
    RPH_PROFILE_FUNC()
    ENQUEUE_RENDER_COMMAND(BeginFrame)([](FFRHICommandList& CommandList) { CommandList.BeginFrame(); });

    FBaseApplication::Tick(DeltaTime);

    MainWindow->SetText(std::to_string(1.0f / DeltaTime));

    ENQUEUE_RENDER_COMMAND(EmptyRender)
    ([this](FFRHICommandList& CommandList) {
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
        CommandList.SetVertexBuffer(VertexBuffer);

        CommandList.SetViewport({0, 0, 0}, {static_cast<float>(MainViewport->GetSize().x),
                                            static_cast<float>(MainViewport->GetSize().y), 1.0f});
        CommandList.SetScissor({0, 0}, {MainViewport->GetSize().x, MainViewport->GetSize().y});

        CommandList.DrawIndexed(IndexBuffer, 0, 0, Cube.Vertices.Size(), 0, Cube.Indices.Size() / 3, 1);

        CommandList.EndRendering();
        CommandList.EndRenderingViewport(MainViewport.Raw());
    });

    ENQUEUE_RENDER_COMMAND(EndFrame)([](FFRHICommandList& CommandList) { CommandList.EndFrame(); });
}

// Not really extern "C" but I use it to mark that this function will be called by an external unit
extern "C" IApplication* GetApplication()
{
    return new EditorApplication();
}
