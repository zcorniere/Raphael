#pragma once

#include "Engine/AssetRegistry/Asset.hxx"
#include "Engine/AssetRegistry/Material.hxx"

class FAssetRegistry
{
public:
    FAssetRegistry();

    Ref<RAsset> LoadAsset(const std::filesystem::path& Path);
    Ref<RMaterial> LoadMaterial(const std::string Name, const FRHIGraphicsPipelineSpecification& Pipeline);

    Ref<RAsset> RegisterMemoryOnlyAsset(Ref<RAsset>& Asset);

    Ref<RAsset> GetAsset(const std::string& Name);
    Ref<RMaterial> GetMaterial(const std::string& Name);

    void UnloadAsset(const std::string& Name);

    void Purge();

    Ref<RAsset> GetCubeAsset() const
    {
        return AssetRegistry.at("Box");
    }

private:
    std::unordered_map<std::string, Ref<RAsset>> AssetRegistry;
    std::unordered_map<std::string, Ref<RMaterial>> MaterialRegistry;
};
