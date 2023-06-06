#include "VulkanRHI/VulkanShaderCompiler.hxx"

#include "Engine/Misc/Utils.hxx"
#include "VulkanRHI/VulkanLoader.hxx"
#include "VulkanRHI/VulkanRHI.hxx"
#include "VulkanRHI/VulkanShaderCompiler.hxx"

#include <shaderc/shaderc.hpp>
#include <spirv_cross.hpp>

DECLARE_LOGGER_CATEGORY(Core, LogVulkanShaderCompiler, Warning)

static uint32 VulkanVersionToShaderc(uint32 Version)
{
    switch (Version) {
        case VK_API_VERSION_1_0:
            return shaderc_env_version_vulkan_1_0;
        case VK_API_VERSION_1_1:
            return shaderc_env_version_vulkan_1_1;
        case VK_API_VERSION_1_2:
            return shaderc_env_version_vulkan_1_2;
        case VK_API_VERSION_1_3:
            return shaderc_env_version_vulkan_1_3;
    }
    checkNoEntry();
    return 0;
}

namespace VulkanRHI
{

static shaderc::CompileOptions GetCompileOption(VulkanShaderCompiler::OptimizationLevel Level)
{
    shaderc::CompileOptions Options;
    Options.SetTargetEnvironment(shaderc_target_env_vulkan, VulkanVersionToShaderc(RHI_VULKAN_VERSION));

    switch (Level) {
        case VulkanShaderCompiler::OptimizationLevel::None:
            Options.SetGenerateDebugInfo();
            Options.SetOptimizationLevel(shaderc_optimization_level_zero);
            break;
        case VulkanShaderCompiler::OptimizationLevel::Size:
            Options.SetOptimizationLevel(shaderc_optimization_level_size);
            break;
        case VulkanShaderCompiler::OptimizationLevel::Performance:
            Options.SetOptimizationLevel(shaderc_optimization_level_performance);
            break;
    }
    return Options;
}

static std::optional<RHIShaderType> GetShaderKind(const std::filesystem::path& File)
{
    const auto stage = File.extension();
    if (stage == ".vert")
        return RHIShaderType::Vertex;
    if (stage == ".frag")
        return RHIShaderType::Fragment;
    if (stage == ".comp")
        return RHIShaderType::Compute;
    else
        return std::nullopt;
}

static shaderc_shader_kind ShaderKindToShaderc(RHIShaderType Kind)
{
    switch (Kind) {
        case RHIShaderType::Compute:
            return shaderc_compute_shader;
        case RHIShaderType::Vertex:
            return shaderc_vertex_shader;
        case RHIShaderType::Fragment:
            return shaderc_fragment_shader;
    }
    checkNoEntry();
}

VulkanShaderCompiler::VulkanShaderCompiler()
{
}

VulkanShaderCompiler::~VulkanShaderCompiler()
{
    m_ShaderCache.clear();
}

void VulkanShaderCompiler::SetOptimizationLevel(OptimizationLevel InLevel)
{
    Level = InLevel;
}

Ref<VulkanShader> VulkanShaderCompiler::Get(std::filesystem::path Path, bool bForceCompile)
{
    {
        std::unique_lock Lock(m_ShaderCacheMutex);
        auto Iter = m_ShaderCache.find(Path.filename().string());
        if (Iter != m_ShaderCache.end() && !bForceCompile) {
            return Ref(Iter->second);
        }
    }

    if (!std::filesystem::exists(Path)) {
        LOG(LogVulkanShaderCompiler, Error, "Shader file not found ! \"{}\"", Path.string().c_str());
        return nullptr;
    }

    std::string FileContent = Utils::readFile(Path);
    std::optional<RHIShaderType> ShaderType = GetShaderKind(Path);
    check(ShaderType);

    Array<uint32> ShaderCode = CompileShader(Path, ShaderType.value(), FileContent);
    VulkanShader::ReflectionData ReflectionData = GenerateReflection(ShaderCode);

    Ref<VulkanShader> ShaderUnit =
        Ref<VulkanShader>::CreateNamed(Path.filename().string(), ShaderType.value(), ShaderCode, ReflectionData);

    {
        std::unique_lock Lock(m_ShaderCacheMutex);
        m_ShaderCache[Path.filename().string()] = ShaderUnit;
    }
    return ShaderUnit;
}

Array<uint32> VulkanShaderCompiler::CompileShader(const std::filesystem::path& Path, RHIShaderType ShaderType,
                                                  const std::string& ShaderCode)
{
    shaderc::Compiler ShaderCompiler;

    shaderc::CompileOptions Options = GetCompileOption(Level);
    shaderc_shader_kind ShaderKind = ShaderKindToShaderc(ShaderType);

    shaderc::PreprocessedSourceCompilationResult PreProcessResult =
        ShaderCompiler.PreprocessGlsl(ShaderCode, ShaderKind, Path.string().c_str(), Options);
    if (PreProcessResult.GetCompilationStatus() != shaderc_compilation_status_success) {
        LOG(LogVulkanShaderCompiler, Error, "Failed to pre-process {}: {}", Path.string().c_str(),
            PreProcessResult.GetErrorMessage());
        return {};
    }

    std::string PreprocessCode(PreProcessResult.begin(), PreProcessResult.end());
    LOG(LogVulkanShaderCompiler, Info, "Pre-process Result \"{}\": {}", Path.string().c_str(), PreprocessCode);

    shaderc::CompilationResult CompilationResult =
        ShaderCompiler.CompileGlslToSpv(PreprocessCode, ShaderKind, Path.string().c_str(), Options);
    if (CompilationResult.GetCompilationStatus() != shaderc_compilation_status_success) {
        LOG(LogVulkanShaderCompiler, Error, "Failed to compile shader \"{}\": {}", Path.string().c_str(),
            CompilationResult.GetErrorMessage());
        return {};
    }
    return {CompilationResult.begin(), CompilationResult.end()};
}

VulkanShader::ReflectionData VulkanShaderCompiler::GenerateReflection(const Array<uint32>& ShaderCode)
{
    LOG(LogVulkanShaderCompiler, Trace, "===========================");
    LOG(LogVulkanShaderCompiler, Trace, " Vulkan Shader Reflection");
    LOG(LogVulkanShaderCompiler, Trace, "===========================");

    spirv_cross::Compiler Compiler(ShaderCode.Raw(), ShaderCode.Size());
    spirv_cross::ShaderResources Resources = Compiler.get_shader_resources();

    VulkanShader::ReflectionData Data;

    LOG(LogVulkanShaderCompiler, Trace, "Push Constant Buffers:");
    for (const spirv_cross::Resource& resource: Resources.push_constant_buffers) {
        auto& bufferType = Compiler.get_type(resource.base_type_id);
        auto bufferSize = (uint32_t)Compiler.get_declared_struct_size(bufferType);
        uint32_t bufferOffset = 0;

        if (!Data.PushConstants.IsEmpty())
            bufferOffset = Data.PushConstants.Back().Offset + Data.PushConstants.Back().Size;

        ShaderResource::PushConstantRange& Range = Data.PushConstants.Emplace();
        Range.Size = bufferSize - bufferOffset;
        Range.Offset = bufferOffset;

        LOG(LogVulkanShaderCompiler, Trace, "  Name: {0}", resource.name);
        LOG(LogVulkanShaderCompiler, Trace, "  Member Count: {0}", bufferType.member_types.size());
        LOG(LogVulkanShaderCompiler, Trace, "  Size: {0}", bufferSize);
    }
    return Data;
}

}    // namespace VulkanRHI
