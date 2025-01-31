#include "Engine/AssetRegistry/AssetRegistry.hxx"

#include "Engine/AssetRegistry/MeshFactory.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogAssetRegistry, Info)

FAssetRegistry::FAssetRegistry()
{
    Ref<RAsset> CubeAsset = MeshFactory::CreateBox({1.0f, 1.0f, 1.0f});
    RegisterMemoryOnlyAsset(CubeAsset);
}

Ref<RAsset> FAssetRegistry::LoadAsset(const std::filesystem::path& Path)
{
    auto Asset = Ref<RAsset>::Create(Path);
    if (Asset->Load()) {
        AssetRegistry[Asset->GetName()] = Asset;
        return Asset;
    }
    return nullptr;
}

Ref<RMaterial> FAssetRegistry::LoadMaterial(const std::string Name, const FRHIGraphicsPipelineSpecification& Pipeline)
{
    Ref<RMaterial> Material = Ref<RMaterial>::CreateNamed(Name, Pipeline);
    if (Material) {
        MaterialRegistry[Name] = Material;
        return Material;
    }
    return nullptr;
}

Ref<RAsset> FAssetRegistry::RegisterMemoryOnlyAsset(Ref<RAsset>& Asset)
{
    if (AssetRegistry.find(Asset->GetName()) == AssetRegistry.end()) {
        AssetRegistry[Asset->GetName()] = Asset;
        return Asset;
    }
    LOG(LogAssetRegistry, Warning, "Asset {:s} already registered", Asset->GetName());
    return nullptr;
}

Ref<RAsset> FAssetRegistry::GetAsset(const std::string& Name)
{
    auto Asset = AssetRegistry.find(Name);
    if (Asset != AssetRegistry.end()) {
        return Asset->second;
    }
    LOG(LogAssetRegistry, Warning, "Asset {:s} not found", Name);
    return nullptr;
}

void FAssetRegistry::UnloadAsset(const std::string& Name)
{
    auto Asset = AssetRegistry.find(Name);
    if (Asset != AssetRegistry.end()) {
        Asset->second->Unload();
        AssetRegistry.erase(Asset);
    }
}

void FAssetRegistry::Purge()
{
    for (auto& Asset: AssetRegistry) {
        Asset.second->Unload();
    }
    AssetRegistry.clear();
    MaterialRegistry.clear();
}
