#pragma once

#include "Engine/GameFramework/World.hxx"
#include <Engine/Core/Application.hxx>

class EditorApplication : public FBaseApplication
{
    RTTI_DECLARE_TYPEINFO(EditorApplication, FBaseApplication)
public:
    EditorApplication();
    ~EditorApplication();

    bool OnEngineInitialization() override;
    void OnEngineDestruction() override;

    void Tick(const double DeltaTime) override;

private:
    Ref<RWorld> World;
};
