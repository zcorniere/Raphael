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

    virtual void WindowEventHandler(FEvent& Event) override;

    void Tick(const double DeltaTime) override;

private:
    bool bShowUI = false;
    Ref<RWorld> World;
};
