#pragma once

#include <Engine/Core/Application.hxx>
#include <Engine/Core/ECS/ECS.hxx>

class EditorApplication : public FBaseApplication
{
    RTTI_DECLARE_TYPEINFO(EditorApplication, FBaseApplication)
public:
    EditorApplication();
    ~EditorApplication();

    bool OnEngineInitialization() override;
    void OnEngineDestruction() override;

    void Tick(const float DeltaTime) override;

private:
    Ref<ecs::RWorld> World = nullptr;
    ecs::FEntity Entity;
    ecs::FEntity CameraEntity;
};
