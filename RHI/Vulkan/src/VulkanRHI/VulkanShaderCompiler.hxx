#pragma once

#include "VulkanRHI/Resources/VulkanShader.hxx"

namespace VulkanRHI
{

class RVulkanShader;

class FVulkanShaderCompiler
{
public:
    /// Which optimization level to use when compiling
    enum class EOptimizationLevel {
        /// No optimization, enable debug symbols
        None,
        /// Optimize for size, no debug symbols
        Size,
        /// Optimize for performance, with debug symbols
        PerfWithDebug,
        /// Optimize for Performance, no debug symbols
        Performance,
    };

private:
    enum class ECompilationStatus {
        None,
        CheckCache,
        Loading,
        PreProcess,
        Compilation,
        Reflection,
        Done,
    };

    struct ShaderCompileResult {
        ECompilationStatus Status = ECompilationStatus::None;
        std::filesystem::path Path;
        ERHIShaderType ShaderType;
        std::string SourceCode;
        TArray<uint32> CompiledCode;
        RVulkanShader::FReflectionData Reflection;
    };

public:
    FVulkanShaderCompiler();
    ~FVulkanShaderCompiler();

    /// @brief Set the optimization level expected when compiling
    ///
    /// Changing the value does not trigger a recompilation
    void SetOptimizationLevel(EOptimizationLevel Level);

    /// @brief Return a shader handle
    /// @param Path The path (internally used as ID) of the shader
    /// @param bForceCompile Should the shader be recompiled regardless of its cached status ?
    Ref<RVulkanShader> Get(std::filesystem::path Path, bool bForceCompile = false, bool bUnitTesting = false);

private:
    Ref<RVulkanShader> CheckCache(ShaderCompileResult& Result);
    bool LoadShaderSourceFile(ShaderCompileResult& Result);
    bool CompileShader(ShaderCompileResult& Result);
    bool GenerateReflection(ShaderCompileResult& Result);

private:
    EOptimizationLevel Level = EOptimizationLevel::None;

    std::mutex m_ShaderCacheMutex;
    std::unordered_map<std::string, WeakRef<RVulkanShader>> m_ShaderCache;
};

}    // namespace VulkanRHI
