#pragma once

#include "Engine/GameFramework/Actor.hxx"

class AOscillator : public AActor
{
    RTTI_DECLARE_TYPEINFO(AOscillator, AActor)
public:
    AOscillator();
    virtual ~AOscillator();

    void Tick(double DeltaTime) override;

public:
    float Multiplier = 1.0f;
    FVector3 Direction = {0.0f, 0.0f, 0.0f};
    FVector3 Maximum = {0.0f, 0.0f, 0.0f};
    FVector3 Minimum = {0.0f, 0.0f, 0.0f};
};
