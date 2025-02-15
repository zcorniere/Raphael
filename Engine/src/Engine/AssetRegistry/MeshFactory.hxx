#pragma once

#include "Engine/AssetRegistry/Asset.hxx"

namespace MeshFactory
{

Ref<RAsset> CreateBox(const FVector3& size);
Ref<RAsset> CreateCapsule(float radius, float height);

}    // namespace MeshFactory
