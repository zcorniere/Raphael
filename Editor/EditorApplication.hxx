#pragma once

#include <Engine/Core/Application.hxx>

class EditorApplication : public FBaseApplication
{
public:
    EditorApplication();
    ~EditorApplication();

    bool OnEngineInitialization() override;
    void OnEngineDestruction() override;

    void Tick(const float DeltaTime) override;

private:
    Ref<RRHIGraphicsPipeline> Pipeline;

    Ref<RRHIBuffer> StorageBuffer;
    Ref<RRHIBuffer> VertexBuffer;
    Ref<RRHIBuffer> IndexBuffer;
};
