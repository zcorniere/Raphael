#include "Engine/AssetRegistry/AssetRegistry.hxx"

#include "Engine/AssetRegistry/MeshFactory.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogAssetRegistry, Info)

FAssetRegistry::FAssetRegistry()
{
    Ref<RAsset> CubeAsset = MeshFactory::CreateBox({1.0f, 1.0f, 1.0f});
    RegisterMemoryOnlyAsset(CubeAsset);

    Ref<RAsset> CapsuleAsset = MeshFactory::CreateCapsule(1.0f, 2.0f);
    RegisterMemoryOnlyAsset(CapsuleAsset);
}

Ref<RAsset> FAssetRegistry::LoadAsset(const std::filesystem::path& Path)
{
    auto Asset = Ref<RAsset>::Create(Path);
    if (Asset->Load())
    {
        AssetRegistry.Insert(Asset->GetName(), Asset);
        AssetRegistryById.Insert(Asset->ID(), Asset);
        return Asset;
    }
    return nullptr;
}

Ref<RAsset> FAssetRegistry::RegisterMemoryOnlyAsset(Ref<RAsset>& Asset)
{
    if (!AssetRegistry.Contains(Asset->GetName()))
    {
        AssetRegistry.Insert(Asset->GetName(), Asset);
        AssetRegistryById.Insert(Asset->ID(), Asset);
        return Asset;
    }
    LOG(LogAssetRegistry, Warning, "Asset {:s} already registered", Asset->GetName());
    return nullptr;
}

Ref<RRHIMaterial> FAssetRegistry::RegisterMemoryOnlyMaterial(Ref<RRHIMaterial>& Material)
{
    if (!MaterialRegistry.Contains(Material->GetName()))
    {
        MaterialRegistryId.Insert(Material->ID(), Material);
        MaterialRegistry.Insert(Material->GetName(), Material);
        return Material;
    }
    LOG(LogAssetRegistry, Warning, "Material {:s} already registered", Material->GetName());
    return nullptr;
}

Ref<RAsset> FAssetRegistry::GetAsset(const std::string& Name) const
{
    const Ref<RAsset>* Asset = AssetRegistry.Find(Name);
    if (Asset)
    {
        return *Asset;
    }
    LOG(LogAssetRegistry, Warning, "Asset {:s} not found", Name);
    return nullptr;
}

Ref<RRHIMaterial> FAssetRegistry::GetMaterial(const std::string& Name) const
{
    const Ref<RRHIMaterial>* Material = MaterialRegistry.Find(Name);
    if (Material)
    {
        return *Material;
    }
    LOG(LogAssetRegistry, Warning, "Material {:s} not found", Name);
    return nullptr;
}

Ref<RAsset> FAssetRegistry::GetAssetByID(uint64 ID) const
{
    const Ref<RAsset>* Asset = AssetRegistryById.Find(ID);
    if (Asset)
    {
        return *Asset;
    }
    LOG(LogAssetRegistry, Warning, "Asset with ID {:d} not found", ID);
    return nullptr;
}

Ref<RRHIMaterial> FAssetRegistry::GetMaterialByID(uint64 ID) const
{
    const Ref<RRHIMaterial>* Material = MaterialRegistryId.Find(ID);
    if (Material)
    {
        return *Material;
    }
    LOG(LogAssetRegistry, Warning, "Material with ID {:d} not found", ID);
    return nullptr;
}

void FAssetRegistry::UnloadAsset(const std::string& Name)
{
    Ref<RAsset>* Asset = AssetRegistry.Find(Name);
    if (Asset)
    {
        (*Asset)->Unload();
        AssetRegistry.Remove(Name);
    }
}

void FAssetRegistry::Purge()
{
    for (auto& [Name, Asset]: AssetRegistry)
    {
        Asset->Unload();
    }
    AssetRegistry.Clear();
    AssetRegistryById.Clear();

    MaterialRegistry.Clear();
    MaterialRegistryId.Clear();
}
