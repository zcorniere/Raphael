#include "Engine/Renderer/Vulkan/VulkanShaderCompiler.hxx"

#include "Engine/Misc/Utils.hxx"
#include "Engine/Renderer/Vulkan/VulkanDynamicRHI.hxx"
#include "Engine/Renderer/Vulkan/VulkanLoader.hxx"

#include <shaderc/shaderc.hpp>

DECLARE_LOGGER_CATEGORY(Core, LogVulkanShaderCompiler, Info)

uint32 VulkanVersionToShaderc(uint32 Version)
{
    switch (Version) {
        case VK_API_VERSION_1_0: return shaderc_env_version_vulkan_1_0;
        case VK_API_VERSION_1_1: return shaderc_env_version_vulkan_1_1;
        case VK_API_VERSION_1_2: return shaderc_env_version_vulkan_1_2;
        case VK_API_VERSION_1_3: return shaderc_env_version_vulkan_1_3;
    }
    checkNoEntry();
    return 0;
}

namespace VulkanRHI
{

shaderc::CompileOptions GetCompileOption(VulkanShaderCompiler::OptimizationLevel Level)
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

RHIShaderType GetShaderKind(const std::filesystem::path &File)
{
    const auto stage = File.extension();
    if (stage == ".vert") return RHIShaderType::Vertex;
    if (stage == ".frag") return RHIShaderType::Fragment;
    if (stage == ".comp") return RHIShaderType::Compute;
    checkNoEntry();
}

shaderc_shader_kind ShaderKindToShaderc(RHIShaderType Kind)
{
    switch (Kind) {
        case RHIShaderType::Compute: return shaderc_compute_shader;
        case RHIShaderType::Vertex: return shaderc_vertex_shader;
        case RHIShaderType::Fragment: return shaderc_fragment_shader;
    }
    checkNoEntry();
}

VulkanShaderCompiler::VulkanShaderCompiler()
{
}

VulkanShaderCompiler::~VulkanShaderCompiler()
{
    s_ShaderCache.clear();
}

void VulkanShaderCompiler::SetOptimizationLevel(OptimizationLevel InLevel)
{
    Level = InLevel;
}

Ref<VulkanShader> VulkanShaderCompiler::Get(std::filesystem::path Path, bool bForceCompile)
{
    (void)bForceCompile;
    shaderc::Compiler ShaderCompiler;

    if (!std::filesystem::exists(Path)) {
        LOG(LogVulkanShaderCompiler, Error, "Shader file not found ! \"{}\"", Path.string().c_str());
        return nullptr;
    }

    shaderc::CompileOptions Options = GetCompileOption(Level);
    RHIShaderType ShaderType = GetShaderKind(Path);
    shaderc_shader_kind ShaderKind = ShaderKindToShaderc(ShaderType);
    std::string FileContent = Utils::readFile(Path);

    shaderc::PreprocessedSourceCompilationResult PreProcessResult =
        ShaderCompiler.PreprocessGlsl(FileContent, ShaderKind, Path.string().c_str(), Options);
    if (PreProcessResult.GetCompilationStatus() != shaderc_compilation_status_success) {
        LOG(LogVulkanShaderCompiler, Error, "Failed to pre-process {}: {}", Path.string().c_str(),
            PreProcessResult.GetErrorMessage());
    }

    std::string PreprocessCode(PreProcessResult.begin(), PreProcessResult.end());
    LOG(LogVulkanShaderCompiler, Debug, "Pre-process Result \"{}\": {}", Path.string().c_str(), PreprocessCode);

    shaderc::CompilationResult CompilationResult =
        ShaderCompiler.CompileGlslToSpv(PreprocessCode, ShaderKind, Path.string().c_str(), Options);
    if (CompilationResult.GetCompilationStatus() != shaderc_compilation_status_success) {
        LOG(LogVulkanShaderCompiler, Error, "Failed to compile shader \"{}\": {}", Path.string().c_str(),
            CompilationResult.GetErrorMessage());
    }

    std::vector<uint32> ShaderCode(CompilationResult.begin(), CompilationResult.end());
    return Ref<VulkanShader>::Create(ShaderType, ShaderCode);
}

}    // namespace VulkanRHI
