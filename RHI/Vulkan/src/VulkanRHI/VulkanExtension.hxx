#pragma once

namespace VulkanRHI
{

/// Declare a new Vulkan extension
class IVulkanExtension
{
protected:
    IVulkanExtension(const char* InExtensionName): ExtensionName(InExtensionName)
    {
    }

public:
    virtual ~IVulkanExtension() = default;

    const char* GetExtensionName() const
    {
        return ExtensionName;
    }

private:
    const char* const ExtensionName = nullptr;
};

/// Interface for Vulkan extensions that require device-level initialization
class IDeviceVulkanExtension : public IVulkanExtension
{
public:
    IDeviceVulkanExtension(const char* InExtensionName): IVulkanExtension(InExtensionName)
    {
    }

    virtual void PreDeviceCreated(VkDeviceCreateInfo& Info)
    {
        (void)Info;
    }
};

/// Interface for Vulkan extensions that require instance-level initialization
class IInstanceVulkanExtension : public IVulkanExtension
{
public:
    IInstanceVulkanExtension(const char* InExtensionName): IVulkanExtension(InExtensionName)
    {
    }

    virtual void PreInstanceCreated(VkInstanceCreateInfo& Info)
    {
        (void)Info;
    }
};

}    // namespace VulkanRHI
