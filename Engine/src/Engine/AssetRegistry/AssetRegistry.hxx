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
    Ref<RAsset> GetAssetByID(uint64 ID) const;
    Ref<RRHIMaterial> GetMaterialByID(uint64 ID) const;

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
    TMap<uint64, Ref<RAsset>> AssetRegistryById;
    TMap<uint64, Ref<RRHIMaterial>> MaterialRegistryId;

    TMap<std::string, Ref<RAsset>> AssetRegistry;
    TMap<std::string, Ref<RRHIMaterial>> MaterialRegistry;
};
