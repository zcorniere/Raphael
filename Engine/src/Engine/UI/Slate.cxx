#include "Engine/UI/Slate.hxx"

#include "Engine/Core/RHI/RHI.hxx"
#include "Engine/Core/RHI/Resources/RHIBuffer.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogSlate, Info)

RSlate::RSlate(Ref<RRHIViewport> InTargetViewport): TargetViewport(InTargetViewport)
{
    check(InTargetViewport);

    FRHIGraphicsPipelineSpecification PipelineConfig;
    PipelineConfig.VertexShader = "UI/VertexShader.vert";
    PipelineConfig.FragmentShader = "UI/VertexShader.frag";
    PipelineConfig.VertexBufferLayouts = {
        {
            .InputMode = EVertexInputMode::PerVertex,
            .Parameter =
                {
                    {.Name = "Position", .Type = EVertexElementType::Float2},
                    {.Name = "UV", .Type = EVertexElementType::Float2},
                    {.Name = "Color", .Type = EVertexElementType::Float4},
                },
        },
    };
    PipelineConfig.Rasterizer = {
        .PolygonMode = EPolygonMode::Fill,
        .CullMode = ECullMode::None,
        .FrontFaceCulling = EFrontFace::CounterClockwise,
    };
    PipelineConfig.AttachmentFormats = {
        .ColorFormats = {TargetViewport->GetBackbuffer()->GetDescription().Format},
        .DepthFormat = std::nullopt,
        .StencilFormat = std::nullopt,
    };
    PipelineConfig.Topology = EPrimitiveTopology::TriangleList;

    GraphicsPipeline = RHI::CreateGraphicsPipeline(PipelineConfig);
}

RSlate::~RSlate()
{
    LOG(LogSlate, Info, "Destroying RSlate instance for viewport {}", TargetViewport->GetName());
}

void RSlate::Rect(float X, float Y, float Width, float Height, FVector4 Color)
{
    auto ToNormalizeDeviceCoordinates = [](FVector2 Position, UVector2 ViewportSize) -> FVector2
    { return FVector2{2.0f * (Position.x / ViewportSize.x) - 1.0f, 2.0f * (Position.y / ViewportSize.y) - 1.0f}; };

    unsigned Index0 = UIVertex.Size() + 1;

    FUIVertex Vertex;
    Vertex.Position = ToNormalizeDeviceCoordinates({X, Y}, TargetViewport->GetSize());
    Vertex.UV = FVector2(0.0f, 0.0f);
    Vertex.Color = Color;
    UIVertex.Add(Vertex);
    UIIndex.Add(UIVertex.Size() - 1);

    Vertex.Position = ToNormalizeDeviceCoordinates({X, Y + Height}, TargetViewport->GetSize());
    Vertex.UV = FVector2(1.0f, 0.0f);
    Vertex.Color = Color;
    UIVertex.Add(Vertex);
    UIIndex.Add(UIVertex.Size() - 1);

    Vertex.Position = ToNormalizeDeviceCoordinates({X + Width, Y}, TargetViewport->GetSize());
    Vertex.UV = FVector2(1.0f, 0.0f);
    Vertex.Color = Color;
    UIVertex.Add(Vertex);
    UIIndex.Add(UIVertex.Size() - 1);

    Vertex.Position = ToNormalizeDeviceCoordinates({X + Width, Y + Height}, TargetViewport->GetSize());
    Vertex.UV = FVector2(1.0f, 1.0f);
    Vertex.Color = Color;
    UIIndex.Add(UIVertex.Size() - 1);
    UIVertex.Add(Vertex);
    UIIndex.Add(UIVertex.Size() - 1);
    UIIndex.Add(Index0);

    DrawCount += 1;
};

void RSlate::Draw()
{
    if (UIVertex.Size() == 0)
    {
        return;
    }
    ENQUEUE_RENDER_COMMAND(DrawUI)
    (
        [this](FFRHICommandList& CommandList)
        {
            FRHIBufferDesc VertexDescription{
                .Size = static_cast<uint32>(UIVertex.Size() * sizeof(FUIVertex)),
                .Stride = sizeof(FUIVertex),
                .Usage = EBufferUsageFlags::VertexBuffer | EBufferUsageFlags::KeepCPUAccessible,
                .ResourceArray = &UIVertex,
                .DebugName = "UI Vertex Buffer",
            };
            if (VertexBuffer && VertexBuffer->GetSize() >= VertexDescription.Size)
            {
                CommandList.CopyResourceArrayToBuffer(&UIVertex, VertexBuffer, 0, 0, VertexDescription.Size);
            }
            else
            {
                VertexBuffer = RHI::CreateBuffer(VertexDescription);
            }

            FRHIBufferDesc IndexDescription{
                .Size = static_cast<uint32>(UIIndex.Size() * sizeof(uint32)),
                .Stride = sizeof(uint32),
                .Usage = EBufferUsageFlags::IndexBuffer | EBufferUsageFlags::KeepCPUAccessible,
                .ResourceArray = &UIIndex,
                .DebugName = "UI Index Buffer",
            };
            if (IndexBuffer && IndexBuffer->GetSize() >= IndexDescription.Size)
            {
                CommandList.CopyResourceArrayToBuffer(&UIIndex, IndexBuffer, 0, 0, IndexDescription.Size);
            }
            else
            {
                IndexBuffer = RHI::CreateBuffer(IndexDescription);
            }

            UVector2 Size = TargetViewport->GetSize();
            TArray<FRHIRenderTarget> ColorTargets = {
                {
                    .Texture = TargetViewport->GetBackbuffer(),
                    .ClearColor = {0.0f, 0.0f, 0.0f, 1.0f},
                    .LoadAction = ERenderTargetLoadAction::Load,
                    .StoreAction = ERenderTargetStoreAction::Store,
                },
            };

            CommandList.SetGraphicsPipeline(GraphicsPipeline);
            CommandList.SetViewport({0, 0, 0}, {static_cast<float>(TargetViewport->GetSize().x),
                                                static_cast<float>(TargetViewport->GetSize().y), 1.0f});
            CommandList.SetScissor({0, 0}, {TargetViewport->GetSize().x, TargetViewport->GetSize().y});

            FRHIRenderPassDescription Description{
                .RenderAreaLocation = {0, 0},
                .RenderAreaSize = Size,
                .ColorTargets = ColorTargets,
                .DepthTarget = std::nullopt,
            };
            CommandList.BeginRendering(Description);

            CommandList.SetVertexBuffer(VertexBuffer);
            CommandList.DrawIndexed(IndexBuffer, 0, 0, UIVertex.Size(), 0, UIIndex.Size(), DrawCount);
            // CommandList.Draw(0, UIVertex.Size(), DrawCount);

            CommandList.EndRendering();

            UIVertex.Clear();
            UIIndex.Clear();
            DrawCount = 0;
        });
}
