#pragma once

#include "Engine/AssetRegistry/Asset.hxx"

class FAssetRegistry
{
public:
    FAssetRegistry();

    Ref<RAsset> LoadAsset(const std::filesystem::path& Path);

    Ref<RAsset> RegisterMemoryOnlyAsset(Ref<RAsset>& Asset);
    Ref<RRHIMaterial> RegisterMemoryOnlyMaterial(Ref<RRHIMaterial>& Material);

    Ref<RAsset> GetAsset(const std::string& Name) const;
    Ref<RRHIMaterial> GetMaterial(const std::string& Name) const;

    void UnloadAsset(const std::string& Name);

    void Purge();

    Ref<RAsset> GetCubeAsset() const
    {
        return AssetRegistry["Box"];
    }
    Ref<RAsset> GetCapsuleAsset() const
    {
        return AssetRegistry["Capsule"];
    }

private:
    TMap<std::string, Ref<RAsset>> AssetRegistry;
    TMap<std::string, Ref<RRHIMaterial>> MaterialRegistry;
};
