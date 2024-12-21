#include "EditorApplication.hxx"
#include "Engine/Math/Vector.hxx"

#include <memory>

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

    FBaseApplication::OnEngineInitialization();

    struct Vertex {
        FVector3 Position;
    };
    TResourceArray<Vertex> Vertices;
    Vertices.Resize(4);
    Vertices[0].Position = {0.5, -0.5, 0};
    Vertices[1].Position = {0.5, 0.5, 0};
    Vertices[2].Position = {-0.5, -0.5, 0};
    Vertices[3].Position = {-0.5, 0.5, 0};

    TResourceArray<uint32> Indices = {0, 1, 2, 2, 1, 3};

    Ref<RRHIBuffer> TmpBuffer = RHI::CreateBuffer(FRHIBufferDesc{
        .Size = Vertices.GetByteSize(),
        .Stride = sizeof(Vertex),
        .Usage = EBufferUsageFlags::VertexBuffer | EBufferUsageFlags::SourceCopy | EBufferUsageFlags::KeepCPUAccessible,
        .ResourceArray = &Vertices,
        .DebugName = "Staging Vertex Buffer",
    });
    Ref<RRHIBuffer> TmpIndexBuffer = RHI::CreateBuffer(FRHIBufferDesc{
        .Size = Indices.GetByteSize(),
        .Stride = sizeof(uint32),
        .Usage = EBufferUsageFlags::IndexBuffer | EBufferUsageFlags::SourceCopy | EBufferUsageFlags::KeepCPUAccessible,
        .ResourceArray = &Indices,
        .DebugName = "Staging Index Buffer",
    });

    srand(time(nullptr));
    StorageBuffer = RHI::CreateBuffer(FRHIBufferDesc{
        .Size = sizeof(FVector4),
        .Stride = sizeof(FVector4),
        .Usage = EBufferUsageFlags::UniformBuffer | EBufferUsageFlags::KeepCPUAccessible,
        .ResourceArray = nullptr,
        .DebugName = "Storage Buffer",
    });

    ENQUEUE_RENDER_COMMAND(FCopyBuffer)
    ([this, TmpBuffer, TmpIndexBuffer](FFRHICommandList& CommandList) {
        VertexBuffer = RHI::CreateBuffer(FRHIBufferDesc{
            .Size = TmpBuffer->GetSize(),
            .Stride = sizeof(Vertex),
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
    VertexBuffer = nullptr;
    IndexBuffer = nullptr;
    StorageBuffer = nullptr;
    Pipeline = nullptr;

    FBaseApplication::OnEngineDestruction();
}

void EditorApplication::Tick(const float DeltaTime)
{
    RPH_PROFILE_FUNC()
    ENQUEUE_RENDER_COMMAND(FBeginFrame)([](FFRHICommandList& CommandList) { CommandList.BeginFrame(); });

    FBaseApplication::Tick(DeltaTime);

    MainWindow->SetText(std::to_string(1.0f / DeltaTime));

    ENQUEUE_RENDER_COMMAND(FEmptyRender)
    ([this, DeltaTime](FFRHICommandList& CommandList) {
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

        static float AccumulatedTime = 0.0f;
        AccumulatedTime += DeltaTime;
        FVector4 Value{std::cos(AccumulatedTime), std::sin(AccumulatedTime), 0.0f, 1.0f};
        TResourceArray<FVector4> Data;
        Data.Emplace(Value);
        CommandList.CopyRessourceArrayToBuffer(&Data, StorageBuffer, 0, 0, Data.GetByteSize());

        Pipeline->SetInput("ColorValue", StorageBuffer);
        CommandList.SetPipeline(Pipeline);
        CommandList.SetVertexBuffer(VertexBuffer);

        CommandList.SetViewport({0, 0, 0}, {static_cast<float>(MainViewport->GetSize().x),
                                            static_cast<float>(MainViewport->GetSize().y), 1.0f});
        CommandList.SetScissor({0, 0}, {MainViewport->GetSize().x, MainViewport->GetSize().y});

        CommandList.DrawIndexed(IndexBuffer, 0, 0, 4, 0, 2, 1);

        CommandList.EndRendering();
        CommandList.EndRenderingViewport(MainViewport.Raw());
    });

    ENQUEUE_RENDER_COMMAND(FEndFrame)([](FFRHICommandList& CommandList) { CommandList.EndFrame(); });
}

// Not really extern "C" but I use it to mark that this function will be called by an external unit
extern "C" IApplication* GetApplication()
{
    return new EditorApplication();
}
