#include "EditorApplication.hxx"

#include <Engine/Core/FrameGraph/FrameGraph.hxx>

DECLARE_LOGGER_CATEGORY(Core, LogApplication, Warning)

EditorApplication::EditorApplication()
{
}

EditorApplication::~EditorApplication()
{
}

bool EditorApplication::OnEngineInitialization()
{
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

    // Create a shader to test the reflection
    RHI::CreateShader("DefaultTriangle.vert", true);
    RHI::CreateShader("DefaultTriangle.frag", true);
    return true;
}

void EditorApplication::OnEngineDestruction()
{
    BaseApplication::OnEngineDestruction();
}

void EditorApplication::Tick(const float DeltaTime)
{
    BaseApplication::Tick(DeltaTime);

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
        .RenderPassSize = MainViewport->GetSize(),
        .Name = "Simple path",
    };
    RHI::BeginRenderPass(Description);
    RHI::EndRenderPass();

    MainViewport->EndDrawViewport();
}
