#pragma once

#include "VulkanRHI/VulkanResources.hxx"

namespace VulkanRHI
{

class VulkanShader;

class VulkanShaderCompiler
{
public:
    /// Which optiomization level to use when compiling
    enum class OptimizationLevel {
        /// No optimization, enable debug symbols
        None,
        /// Optimize for size, no debug symbols
        Size,
        /// Optimize for Performance, no debug symbols
        Performance,
    };

private:
    enum class CompilationStatus {
        None,
        CheckCache,
        Loading,
        PreProcess,
        Compilation,
        Reflection,
        Done,
    };

    struct ShaderCompileResult {
        CompilationStatus Status = CompilationStatus::None;
        std::filesystem::path Path;
        RHIShaderType ShaderType;
        std::string SourceCode;
        Array<uint32> CompiledCode;
        VulkanShader::ReflectionData Reflection;
    };

public:
    VulkanShaderCompiler();
    ~VulkanShaderCompiler();

    /// @brief Set the optimization level expected when compiling
    ///
    /// Changing the value does not trigger a recompilation
    void SetOptimizationLevel(OptimizationLevel Level);

    /// @brief Return a shader handle
    /// @param Path The path (internally used as ID) of the shader
    /// @param bForceCompile Should the shader be recompiled regardless of its cached status ?
    Ref<VulkanShader> Get(std::filesystem::path Path, bool bForceCompile = false);

private:
    Ref<VulkanShader> CheckCache(ShaderCompileResult& Result);
    bool LoadShaderSourceFile(ShaderCompileResult& Result);
    bool CompileShader(ShaderCompileResult& Result);
    bool GenerateReflection(ShaderCompileResult& Result);

private:
    OptimizationLevel Level;

    std::mutex m_ShaderCacheMutex;
    std::unordered_map<std::string, WeakRef<VulkanShader>> m_ShaderCache;
};

}    // namespace VulkanRHI
