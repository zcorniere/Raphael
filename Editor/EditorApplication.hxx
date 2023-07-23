#pragma once

#include <Engine/Core/Application.hxx>

class EditorApplication : public BaseApplication
{
public:
    EditorApplication();
    ~EditorApplication();

    bool OnEngineInitialization() override;
    void OnEngineDestruction() override;

    void Tick(const float DeltaTime) override;
};
