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

public:
    VulkanShaderCompiler();
    ~VulkanShaderCompiler();

    void SetOptimizationLevel(OptimizationLevel Level);

    Ref<VulkanShader> Get(std::filesystem::path Path, bool bForceCompile = false);

private:
    Array<uint32> CompileShader(const std::filesystem::path& Path, RHIShaderType ShaderType,
                                const std::string& ShaderCode);
    VulkanShader::ReflectionData GenerateReflection(const Array<uint32>& ShaderCode);

private:
    OptimizationLevel Level;

    std::mutex m_ShaderCacheMutex;
    std::unordered_map<std::string, WeakRef<VulkanShader>> m_ShaderCache;
};

}    // namespace VulkanRHI
