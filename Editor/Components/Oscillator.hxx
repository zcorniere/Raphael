#pragma once

struct FOscillator {
    RTTI_DECLARE_TYPEINFO_MINIMAL(FOscillator);

public:
    float Multiplier = 1.0f;
    FVector3 Direction = {0.0f, 0.0f, 0.0f};
    FVector3 Maximum = {0.0f, 0.0f, 0.0f};
    FVector3 Minimum = {0.0f, 0.0f, 0.0f};
};
