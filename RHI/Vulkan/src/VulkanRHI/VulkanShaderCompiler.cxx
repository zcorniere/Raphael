#include "VulkanRHI/VulkanShaderCompiler.hxx"

#include "Engine/Misc/Utils.hxx"
#include "VulkanRHI/VulkanLoader.hxx"
#include "VulkanRHI/VulkanRHI.hxx"
#include "VulkanRHI/VulkanShaderCompiler.hxx"

#include "VulkanShaderCompiler.hxx"
#include <shaderc/shaderc.hpp>
#include <spirv_cross.hpp>

DECLARE_LOGGER_CATEGORY(Core, LogVulkanShaderCompiler, Info)

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

namespace Utils
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

    static std::optional<EVertexElementType> SPRIVTypeToVertexElement(const spirv_cross::SPIRType& Type)
    {
#define SPIRV_CONVERT_VEC(SpirvType, RaphTypePrefix)                                        \
    case spirv_cross::SpirvType: {                                                          \
        if (!verifyMsg(Type.columns == 1, "Shader stage IO matrices is not supported !")) { \
            return std::nullopt;                                                            \
        }                                                                                   \
        switch (Type.vecsize) {                                                             \
            case 1:                                                                         \
                return EVertexElementType::RaphTypePrefix##1;                               \
            case 2:                                                                         \
                return EVertexElementType::RaphTypePrefix##2;                               \
            case 3:                                                                         \
                return EVertexElementType::RaphTypePrefix##3;                               \
            case 4:                                                                         \
                return EVertexElementType::RaphTypePrefix##4;                               \
        }                                                                                   \
        checkNoEntry();                                                                     \
    }

        switch (Type.basetype) {
            SPIRV_CONVERT_VEC(SPIRType::Int, Int)
            SPIRV_CONVERT_VEC(SPIRType::UInt, Uint)
            SPIRV_CONVERT_VEC(SPIRType::Float, Float)
            default:
                break;
        }
        return std::nullopt;
    }

#undef SPIRV_CONVERT_VEC
}    // namespace Utils

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
    Ref<VulkanShader> ShaderUnit = nullptr;
    ShaderCompileResult Result{
        .Path = Path,
    };
    if (!bForceCompile) {
        ShaderUnit = CheckCache(Result);
        if (ShaderUnit) {
            return ShaderUnit;
        }
    }

    if (!LoadShaderSourceFile(Result)) {
        LOG(LogVulkanShaderCompiler, Error, "Error at stage {} for shader {}!", magic_enum::enum_name(Result.Status),
            Path.string());
        return nullptr;
    }

    if (!CompileShader(Result)) {
        LOG(LogVulkanShaderCompiler, Error, "Error at stage {} for shader {}!", magic_enum::enum_name(Result.Status),
            Path.string());
        return nullptr;
    }
    if (!GenerateReflection(Result)) {
        LOG(LogVulkanShaderCompiler, Error, "Error at stage {} for shader {}!", magic_enum::enum_name(Result.Status),
            Path.string());
        return nullptr;
    }

    ShaderUnit = Ref<VulkanShader>::CreateNamed(Path.filename().string(), Result.ShaderType, Result.CompiledCode,
                                                Result.Reflection);
    Result.Status = CompilationStatus::Done;
    {
        std::unique_lock Lock(m_ShaderCacheMutex);
        m_ShaderCache[Path.filename().string()] = ShaderUnit;
    }
    return ShaderUnit;
}

Ref<VulkanShader> VulkanShaderCompiler::CheckCache(ShaderCompileResult& Result)
{
    Result.Status = CompilationStatus::CheckCache;
    std::unique_lock Lock(m_ShaderCacheMutex);
    auto Iter = m_ShaderCache.find(Result.Path.filename().string());
    if (Iter != m_ShaderCache.end()) {
        return Ref(Iter->second);
    }
    return nullptr;
}

bool VulkanShaderCompiler::LoadShaderSourceFile(ShaderCompileResult& Result)
{
    Result.Status = CompilationStatus::Loading;

    std::optional<RHIShaderType> ShaderType = Utils::GetShaderKind(Result.Path);
    if (!ShaderType.has_value()) {
        LOG(LogVulkanShaderCompiler, Error, "Can't recognise the shader type ! {}", Result.Path.filename().string());
        return false;
    }
    Result.SourceCode = ::Utils::readFile(Result.Path);
    if (Result.SourceCode.empty()) {
        LOG(LogVulkanShaderCompiler, Error, "Shader file not found ! \"{}\"", Result.Path.string().c_str());
        return false;
    }
    return true;
}

bool VulkanShaderCompiler::CompileShader(ShaderCompileResult& Result)
{
    shaderc::Compiler ShaderCompiler;

    shaderc::CompileOptions Options = Utils::GetCompileOption(Level);
    shaderc_shader_kind ShaderKind = Utils::ShaderKindToShaderc(Result.ShaderType);

    Result.Status = CompilationStatus::PreProcess;
    shaderc::PreprocessedSourceCompilationResult PreProcessResult =
        ShaderCompiler.PreprocessGlsl(Result.SourceCode, ShaderKind, Result.Path.string().c_str(), Options);
    if (PreProcessResult.GetCompilationStatus() != shaderc_compilation_status_success) {
        LOG(LogVulkanShaderCompiler, Error, "Failed to pre-process: {}", PreProcessResult.GetErrorMessage());
        return false;
    }

    std::string PreprocessCode(PreProcessResult.begin(), PreProcessResult.end());
    LOG(LogVulkanShaderCompiler, Trace, "Pre-process Result \"{}\": {}", Result.Path.string().c_str(), PreprocessCode);

    Result.Status = CompilationStatus::Compilation;
    shaderc::CompilationResult CompilationResult =
        ShaderCompiler.CompileGlslToSpv(PreprocessCode, ShaderKind, Result.Path.string().c_str(), Options);
    if (CompilationResult.GetCompilationStatus() != shaderc_compilation_status_success) {
        LOG(LogVulkanShaderCompiler, Error, "Failed to compile shader \"{}\": {}", Result.Path.string().c_str(),
            CompilationResult.GetErrorMessage());
        return false;
    }
    Result.CompiledCode = Array(CompilationResult.begin(), CompilationResult.end());
    return true;
}

bool GetStageReflection(const spirv_cross::SmallVector<spirv_cross::Resource>& ResourceStage,
                        const spirv_cross::Compiler& Compiler, Array<ShaderResource::StageIO>& StageIO)
{
    StageIO.Reserve(ResourceStage.size());
    for (const spirv_cross::Resource& resource: ResourceStage) {
        ShaderResource::StageIO& OutResource = StageIO.Emplace();

        auto ResourceType = Compiler.get_type(resource.base_type_id);
        std::optional<EVertexElementType> ElementType = Utils::SPRIVTypeToVertexElement(ResourceType);
        if (!ElementType.has_value()) {
            return false;
        }

        OutResource.Name = resource.name;
        OutResource.Type = ElementType.value();
        OutResource.Location = Compiler.get_decoration(resource.id, spv::DecorationLocation);

        LOG(LogVulkanShaderCompiler, Info, "- Location {}:", OutResource.Location);
        LOG(LogVulkanShaderCompiler, Info, "    Name: {}", OutResource.Name);
        LOG(LogVulkanShaderCompiler, Info, "    Type: {}", magic_enum::enum_name(OutResource.Type));
    }
    return true;
}

bool VulkanShaderCompiler::GenerateReflection(ShaderCompileResult& Result)
{
    Result.Status = CompilationStatus::Reflection;
    LOG(LogVulkanShaderCompiler, Info, "===========================");
    LOG(LogVulkanShaderCompiler, Info, " Vulkan Shader Reflection");
    LOG(LogVulkanShaderCompiler, Info, " {} ", Result.Path.string());
    LOG(LogVulkanShaderCompiler, Info, "===========================");

    spirv_cross::Compiler compiler(Result.CompiledCode.Raw(), Result.CompiledCode.Size());
    spirv_cross::ShaderResources resources = compiler.get_shader_resources();

    LOG(LogVulkanShaderCompiler, Info, "Stage input:");
    if (!GetStageReflection(resources.stage_inputs, compiler, Result.Reflection.StageInput)) {
        return false;
    }

    LOG(LogVulkanShaderCompiler, Info, "Stage output:");
    if (!GetStageReflection(resources.stage_outputs, compiler, Result.Reflection.StageOutput)) {
        return false;
    }

    LOG(LogVulkanShaderCompiler, Info, "Push Constant Buffers:");
    for (const spirv_cross::Resource& resource: resources.push_constant_buffers) {
        auto bufferType = compiler.get_type(resource.base_type_id);
        auto bufferSize = (uint32_t)compiler.get_declared_struct_size(bufferType);
        uint32_t bufferOffset = 0;

        if (!Result.Reflection.PushConstants.IsEmpty())
            bufferOffset = Result.Reflection.PushConstants.Back().Offset + Result.Reflection.PushConstants.Back().Size;

        ShaderResource::PushConstantRange& Range = Result.Reflection.PushConstants.Emplace();
        Range.Size = bufferSize - bufferOffset;
        Range.Offset = bufferOffset;

        LOG(LogVulkanShaderCompiler, Info, "  Name: {0}", resource.name);
        LOG(LogVulkanShaderCompiler, Info, "  Member Count: {0}", bufferType.member_types.size());
        LOG(LogVulkanShaderCompiler, Info, "  Size: {0}", bufferSize);
    }
    return true;
}

}    // namespace VulkanRHI
