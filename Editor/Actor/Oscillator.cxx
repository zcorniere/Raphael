#include "Oscillator.hxx"

#include "Engine/Core/Engine.hxx"

AOscillator::AOscillator()
{
    Minimum = {0.0f, 0.0f, -6.0f};
    Maximum = {0.0f, 0.0f, 6.0f};
    Direction = {0.0f, 0.0f, 1.0f};

    Ref<RAsset> CubeAsset = GEngine->AssetRegistry.GetCubeAsset();
    CubeAsset->LoadOnGPU();
    GetMesh()->SetAsset(CubeAsset);
    GetMesh()->Material = GEngine->AssetRegistry.GetMaterial("Cube Material");
}

AOscillator::~AOscillator()
{
}

void AOscillator::Tick(double DeltaTime)
{
    Super::Tick(DeltaTime);

    const FVector3 Delta = Direction * Multiplier * float(DeltaTime);
    const FVector3 NewLocation = GetRootComponent()->GetRelativeTransform().GetLocation() + Delta;
    if (NewLocation.x > Maximum.x || NewLocation.x < Minimum.x) {
        Direction.x *= -1;
    }
    if (NewLocation.y > Maximum.y || NewLocation.y < Minimum.y) {
        Direction.y *= -1;
    }
    if (NewLocation.z > Maximum.z || NewLocation.z < Minimum.z) {
        Direction.z *= -1;
    }

    SetActorLocation(NewLocation);
}
