#include "VulkanRHI/VulkanShaderCompiler.hxx"

#include "Engine/Misc/DataLocation.hxx"
#include "Engine/Misc/Utils.hxx"
#include "VulkanRHI/VulkanLoader.hxx"
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
    class ShaderIncluder : public shaderc::CompileOptions::IncluderInterface
    {
        shaderc_include_result* GetInclude(const char* requestedSource, shaderc_include_type type,
                                           const char* requestingSource, size_t includeDepth) override final
        {
            (void)includeDepth;

            std::filesystem::path FileName;
            switch (type) {
                case shaderc_include_type_relative: {
                    const std::filesystem::path msg(requestingSource);
                    FileName = msg.parent_path() / requestedSource;
                } break;
                case shaderc_include_type_standard: {
                    FileName = DataLocationFinder::GetShaderPath() / requestedSource;
                } break;
            }

            std::string* Container = new std::string[2];
            Container[0] = FileName.string();
            Container[1] = ::Utils::ReadFile(FileName);

            if (Container[1].empty()) {
                LOG(LogVulkanShaderCompiler, Error, "Failed to read include file: {}", FileName.string());
                delete[] Container;
                return nullptr;
            }

            shaderc_include_result* const Result = new shaderc_include_result;
            Result->user_data = Container;
            Result->source_name = Container[0].data();
            Result->source_name_length = Container[0].size();
            Result->content = Container[1].data();
            Result->content_length = Container[1].size();
            return Result;
        }
        void ReleaseInclude(shaderc_include_result* data) override final
        {
            std::string* const Container = static_cast<std::string*>(data->user_data);
            delete[] Container;

            delete data;
        }
    };

    static shaderc::CompileOptions GetCompileOption(FVulkanShaderCompiler::EOptimizationLevel Level)
    {
        shaderc::CompileOptions Options;
        Options.SetTargetEnvironment(shaderc_target_env_vulkan, VulkanVersionToShaderc(RHI_VULKAN_VERSION));
        Options.SetPreserveBindings(true);

        switch (Level) {
            case FVulkanShaderCompiler::EOptimizationLevel::None:
                Options.SetGenerateDebugInfo();
                Options.SetOptimizationLevel(shaderc_optimization_level_zero);
                break;
            case FVulkanShaderCompiler::EOptimizationLevel::Size:
                Options.SetOptimizationLevel(shaderc_optimization_level_size);
                break;
            case FVulkanShaderCompiler::EOptimizationLevel::PerfWithDebug:
                Options.SetOptimizationLevel(shaderc_optimization_level_performance);
                Options.SetGenerateDebugInfo();
                break;
            case FVulkanShaderCompiler::EOptimizationLevel::Performance:
                Options.SetOptimizationLevel(shaderc_optimization_level_performance);
                break;
        }
        return Options;
    }

    static std::optional<ERHIShaderType> GetShaderKind(const std::filesystem::path& File)
    {
        const auto stage = File.extension();
        if (stage == ".vert")
            return ERHIShaderType::Vertex;
        if (stage == ".frag")
            return ERHIShaderType::Fragment;
        if (stage == ".comp")
            return ERHIShaderType::Compute;
        else
            return std::nullopt;
    }

    static shaderc_shader_kind ShaderTypeToShaderc(ERHIShaderType Kind)
    {
        switch (Kind) {
            case ERHIShaderType::Compute:
                return shaderc_compute_shader;
            case ERHIShaderType::Vertex:
                return shaderc_vertex_shader;
            case ERHIShaderType::Fragment:
                return shaderc_fragment_shader;
        }
        checkNoEntry();
    }

    static std::optional<EVertexElementType> SPRIVTypeToVertexElement(const spirv_cross::SPIRType& Type)
    {
#define SPIRV_CONVERT_VEC(SpirvType, RaphTypePrefix)                                        \
    case spirv_cross::SpirvType: {                                                          \
        if (!ensureMsg(Type.columns == 1, "Shader stage IO matrices is not supported !")) { \
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

FVulkanShaderCompiler::FVulkanShaderCompiler()
{
}

FVulkanShaderCompiler::~FVulkanShaderCompiler()
{
    m_ShaderCache.clear();
}

void FVulkanShaderCompiler::SetOptimizationLevel(EOptimizationLevel InLevel)
{
    Level = InLevel;
}

Ref<RVulkanShader> FVulkanShaderCompiler::Get(std::filesystem::path Path, bool bForceCompile)
try {
    RPH_PROFILE_FUNC()

    Ref<RVulkanShader> ShaderUnit = nullptr;
    ShaderCompileResult Result{
        .Path = Path,
    };
    if (!bForceCompile) {
        ShaderUnit = CheckCache(Result);
        if (ShaderUnit) {
            LOG(LogVulkanShaderCompiler, Trace, "Cache hit with shader: {:s} !", Path.filename().string());
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

    ShaderUnit = Ref<RVulkanShader>::CreateNamed(Path.filename().string(), Result.ShaderType, Result.CompiledCode,
                                                 Result.Reflection);
    Result.Status = ECompilationStatus::Done;
    {
        std::unique_lock Lock(m_ShaderCacheMutex);
        m_ShaderCache[Path.filename().string()] = ShaderUnit;
    }
    return ShaderUnit;
} catch (const spirv_cross::CompilerError& exception) {
    LOG(LogVulkanShaderCompiler, Error, "Error during compilation of shader {}: {}", Path.filename().string(),
        exception.what());
    return nullptr;
};

Ref<RVulkanShader> FVulkanShaderCompiler::CheckCache(ShaderCompileResult& Result)
{
    Result.Status = ECompilationStatus::CheckCache;
    std::unique_lock Lock(m_ShaderCacheMutex);
    auto Iter = m_ShaderCache.find(Result.Path.filename().string());
    if (Iter != m_ShaderCache.end()) {
        return Ref(Iter->second);
    }
    return nullptr;
}

bool FVulkanShaderCompiler::LoadShaderSourceFile(ShaderCompileResult& Result)
{
    RPH_PROFILE_FUNC()

    Result.Status = ECompilationStatus::Loading;

    std::optional<ERHIShaderType> ShaderType = Utils::GetShaderKind(Result.Path);
    if (!ShaderType.has_value()) {
        LOG(LogVulkanShaderCompiler, Error, "Can't recognise the shader type ! {}", Result.Path.filename().string());
        return false;
    }
    Result.ShaderType = ShaderType.value();
    Result.SourceCode = ::Utils::ReadFile(Result.Path);
    if (Result.SourceCode.empty()) {
        LOG(LogVulkanShaderCompiler, Error, "Shader file not found ! \"{}\"", Result.Path.string().c_str());
        return false;
    }
    return true;
}

bool FVulkanShaderCompiler::CompileShader(ShaderCompileResult& Result)
{
    RPH_PROFILE_FUNC()

    shaderc::Compiler ShaderCompiler;

    LOG(LogVulkanShaderCompiler, Trace, "Optimization Level: {}", magic_enum::enum_name(Level));

    shaderc_shader_kind ShaderKind = Utils::ShaderTypeToShaderc(Result.ShaderType);
    shaderc::CompileOptions Options = Utils::GetCompileOption(Level);
    Options.SetIncluder(std::make_unique<Utils::ShaderIncluder>());

    Result.Status = ECompilationStatus::PreProcess;
    shaderc::PreprocessedSourceCompilationResult PreProcessResult =
        ShaderCompiler.PreprocessGlsl(Result.SourceCode, ShaderKind, Result.Path.string().c_str(), Options);
    if (PreProcessResult.GetCompilationStatus() != shaderc_compilation_status_success) {
        LOG(LogVulkanShaderCompiler, Error, "Failed to pre-process: {}", PreProcessResult.GetErrorMessage());
        return false;
    }

    std::string PreprocessCode(PreProcessResult.begin(), PreProcessResult.end());
    LOG(LogVulkanShaderCompiler, Trace, "Pre-process Result \"{}\":\n{}", Result.Path.string().c_str(), PreprocessCode);

    Result.Status = ECompilationStatus::Compilation;
    shaderc::CompilationResult CompilationResult =
        ShaderCompiler.CompileGlslToSpv(PreprocessCode, ShaderKind, Result.Path.string().c_str(), Options);
    if (CompilationResult.GetCompilationStatus() != shaderc_compilation_status_success) {
        LOG(LogVulkanShaderCompiler, Error, "Failed to compile shader \"{}\": {}", Result.Path.string().c_str(),
            CompilationResult.GetErrorMessage());
        return false;
    }
    Result.CompiledCode = TArray(CompilationResult.begin(), CompilationResult.end());
    return true;
}

static bool GetStageReflection(const spirv_cross::SmallVector<spirv_cross::Resource>& ResourceStage,
                               const spirv_cross::Compiler& Compiler, TArray<ShaderResource::FStageIO>& StageIO)
{
    StageIO.Reserve(ResourceStage.size());
    for (const spirv_cross::Resource& resource: ResourceStage) {
        ShaderResource::FStageIO& OutResource = StageIO.Emplace();

        const spirv_cross::SPIRType& ResourceType = Compiler.get_type(resource.base_type_id);
        std::optional<EVertexElementType> ElementType = Utils::SPRIVTypeToVertexElement(ResourceType);
        if (!ElementType.has_value()) {
            return false;
        }

        OutResource.Name = resource.name;
        OutResource.Type = ElementType.value();
        OutResource.Binding = Compiler.get_decoration(resource.id, spv::DecorationBinding);
        OutResource.Location = Compiler.get_decoration(resource.id, spv::DecorationLocation);
        OutResource.Offset = Compiler.get_decoration(resource.id, spv::DecorationOffset);
    }
    std::sort(StageIO.begin(), StageIO.end(), [](const ShaderResource::FStageIO& A, const ShaderResource::FStageIO& B) {
        return A.Location < B.Location;
    });
    for (const ShaderResource::FStageIO& Resource: StageIO) {
        LOG(LogVulkanShaderCompiler, Info, "- {}", Resource);
    }
    return true;
}

static ::RTTI::FParameter RecursiveTypeDescription(const spirv_cross::Compiler& Compiler,
                                                   spirv_cross::TypeID BaseTypeID, spirv_cross::TypeID ID, uint32 Index)
{
    ::RTTI::FParameter Parameter;

    const spirv_cross::SPIRType& Type = Compiler.get_type(ID);
    const spirv_cross::SPIRType& BaseType = Compiler.get_type(BaseTypeID);
    Parameter.Name = std::string(Compiler.get_member_name(BaseTypeID, Index));

    switch (Type.basetype) {
        case spirv_cross::SPIRType::Struct:
            Parameter.Type = ::RTTI::EParameterType::Struct;
            break;
        case spirv_cross::SPIRType::Int:
            Parameter.Type = ::RTTI::EParameterType::Int32;
            break;
        case spirv_cross::SPIRType::UInt:
            Parameter.Type = ::RTTI::EParameterType::Uint32;
            break;
        case spirv_cross::SPIRType::Float:
            Parameter.Type = ::RTTI::EParameterType::Float;
            break;
        default:
            Parameter.Type = ::RTTI::EParameterType::Invalid;
            break;
    }
    if (Type.member_types.size()) {
        Parameter.Size = Compiler.get_declared_struct_size(Type);
    } else {
        Parameter.Size = sizeof(uint32);    // If there is not subtype, that means it is a scalar type, so uint32
    }

    Parameter.Offset = Compiler.type_struct_member_offset(BaseType, Index);
    Parameter.Columns = Type.columns;
    Parameter.Rows = Type.vecsize;

    BaseTypeID = Compiler.get_type(ID).self;
    for (uint32 i = 0; i < Type.member_types.size(); ++i) {

        Parameter.Members.Add(RecursiveTypeDescription(Compiler, BaseTypeID, Type.member_types[i], i));
    }
    return Parameter;
}

static bool GetPushConstantReflection(const spirv_cross::Compiler& Compiler,
                                      const spirv_cross::SmallVector<spirv_cross::Resource>& PushConstants,
                                      std::optional<ShaderResource::FPushConstantRange>& OutPushConstant)
{
    if (PushConstants.size() == 0) {
        // Nothing to do, technically still a success.
        return true;
    }
    if (!ensureAlways(PushConstants.size() == 1)) {
        return false;
    }

    const spirv_cross::Resource& resource = PushConstants.front();
    const spirv_cross::SPIRType& Type = Compiler.get_type(resource.base_type_id);

    OutPushConstant = ShaderResource::FPushConstantRange{
        .Offset = Compiler.type_struct_member_offset(Type, 0),
        .Size = static_cast<uint32>(Compiler.get_declared_struct_size(Type)),
        .Parameter = RecursiveTypeDescription(Compiler, resource.base_type_id, resource.base_type_id, 0),
    };
    LOG(LogVulkanShaderCompiler, Info, "  {}", *OutPushConstant);

    return true;
}

static VkWriteDescriptorSet GetWriteDescriptorSet(VkDescriptorType Type, uint32 Binding, uint32 Count)
{
    return {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = VK_NULL_HANDLE,
        .dstBinding = Binding,
        .dstArrayElement = 0,
        .descriptorCount = Count,
        .descriptorType = Type,
        .pImageInfo = nullptr,
        .pBufferInfo = nullptr,
        .pTexelBufferView = nullptr,
    };
}

static bool GetStorageBufferReflection(const spirv_cross::Compiler& Compiler,
                                       const spirv_cross::SmallVector<spirv_cross::Resource>& ShaderStorageBuffers,
                                       TArray<ShaderResource::FStorageBuffer>& OutStorageBuffers,
                                       std::unordered_map<std::string, VkWriteDescriptorSet>& WriteDescriptorSet)
{
    for (const spirv_cross::Resource& resource: ShaderStorageBuffers) {
        const spirv_cross::SPIRType& Type = Compiler.get_type(resource.base_type_id);

        ShaderResource::FStorageBuffer& Buffer = OutStorageBuffers.Emplace();
        Buffer.Set = Compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        Buffer.Binding = Compiler.get_decoration(resource.id, spv::DecorationBinding);
        Buffer.Parameter.Name = resource.name;
        Buffer.Parameter.Size = Compiler.get_declared_struct_size(Type);
        Buffer.Parameter.Type = RTTI::EParameterType::Struct;
        Buffer.Parameter.Offset = 0;
        Buffer.Parameter.Rows = Type.vecsize;
        Buffer.Parameter.Columns = Type.columns;

        for (unsigned int i = 0; i < Type.member_types.size(); ++i) {
            spirv_cross::TypeID ID = Type.member_types[i];
            Buffer.Parameter.Members.Add(RecursiveTypeDescription(Compiler, resource.base_type_id, ID, i));
        }
        LOG(LogVulkanShaderCompiler, Info, "  {}", Buffer);

        WriteDescriptorSet[Buffer.Parameter.Name] =
            GetWriteDescriptorSet(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, Buffer.Binding, 1);
    }
    return true;
}

static bool GetUniformBufferReflection(const spirv_cross::Compiler& Compiler,
                                       const spirv_cross::SmallVector<spirv_cross::Resource>& ShaderUniformBuffers,
                                       TArray<ShaderResource::FUniformBuffer>& OutUniformBuffers,
                                       std::unordered_map<std::string, VkWriteDescriptorSet>& WriteDescriptorSet)
{
    for (const spirv_cross::Resource& resource: ShaderUniformBuffers) {
        const spirv_cross::SPIRType& Type = Compiler.get_type(resource.base_type_id);

        ShaderResource::FUniformBuffer& Buffer = OutUniformBuffers.Emplace();
        Buffer.Set = Compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        Buffer.Binding = Compiler.get_decoration(resource.id, spv::DecorationBinding);
        Buffer.Parameter.Name = resource.name;
        Buffer.Parameter.Size = Compiler.get_declared_struct_size(Type);
        Buffer.Parameter.Type = RTTI::EParameterType::Struct;
        Buffer.Parameter.Offset = 0;
        Buffer.Parameter.Rows = Type.vecsize;
        Buffer.Parameter.Columns = Type.columns;

        for (unsigned int i = 0; i < Type.member_types.size(); ++i) {
            spirv_cross::TypeID ID = Type.member_types[i];
            Buffer.Parameter.Members.Add(RecursiveTypeDescription(Compiler, resource.base_type_id, ID, i));
        }
        LOG(LogVulkanShaderCompiler, Info, "  {}", Buffer);

        WriteDescriptorSet[Buffer.Parameter.Name] =
            GetWriteDescriptorSet(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Buffer.Binding, 1);
    }

    return true;
}

bool FVulkanShaderCompiler::GenerateReflection(ShaderCompileResult& Result)
{
    RPH_PROFILE_FUNC()

    Result.Status = ECompilationStatus::Reflection;
    LOG(LogVulkanShaderCompiler, Info, "===========================");
    LOG(LogVulkanShaderCompiler, Info, " Vulkan Shader Reflection - {}", magic_enum::enum_name(Result.ShaderType));
    LOG(LogVulkanShaderCompiler, Info, " {} ", Result.Path.string());
    LOG(LogVulkanShaderCompiler, Info, "===========================");

    spirv_cross::Compiler compiler(Result.CompiledCode.Raw(), Result.CompiledCode.Size());
    const spirv_cross::ShaderResources& resources = compiler.get_shader_resources();

    LOG(LogVulkanShaderCompiler, Info, "Stage input:{}", resources.stage_inputs.empty() ? " None" : "");
    if (!GetStageReflection(resources.stage_inputs, compiler, Result.Reflection.StageInput)) {
        return false;
    }

    LOG(LogVulkanShaderCompiler, Info, "Stage output:{}", resources.stage_outputs.empty() ? " None" : "");
    if (!GetStageReflection(resources.stage_outputs, compiler, Result.Reflection.StageOutput)) {
        return false;
    }

    LOG(LogVulkanShaderCompiler, Info, "Push Constant Buffers:{}",
        resources.push_constant_buffers.empty() ? " None" : "");
    if (!GetPushConstantReflection(compiler, resources.push_constant_buffers, Result.Reflection.PushConstants)) {
        return false;
    }

    LOG(LogVulkanShaderCompiler, Info, "Storage Buffers:{}", resources.storage_buffers.empty() ? " None" : "");
    if (!GetStorageBufferReflection(compiler, resources.storage_buffers, Result.Reflection.StorageBuffers,
                                    Result.Reflection.WriteDescriptorSet)) {
        return false;
    }
    LOG(LogVulkanShaderCompiler, Info, "Uniform Buffers:{}", resources.uniform_buffers.empty() ? " None" : "");
    if (!GetUniformBufferReflection(compiler, resources.uniform_buffers, Result.Reflection.UniformBuffers,
                                    Result.Reflection.WriteDescriptorSet)) {
        return false;
    }

    return true;
}

}    // namespace VulkanRHI
