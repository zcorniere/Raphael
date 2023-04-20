#pragma once

#include "RHI/Vulkan/VulkanResources.hxx"

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
    OptimizationLevel Level;

    std::mutex m_ShaderCacheMutex;
    std::unordered_map<std::string, Ref<VulkanShader>> m_ShaderCache;
};

}    // namespace VulkanRHI
