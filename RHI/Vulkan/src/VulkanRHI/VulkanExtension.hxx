#pragma once

namespace VulkanRHI
{

struct FOptionalExtensionStatus {
    bool Maintenance5 = false;
};

/// Declare a new Vulkan extension
class IVulkanExtension
{
protected:
    IVulkanExtension(const char* InExtensionName, bool bInRequired)
        : bRequired(bInRequired), ExtensionName(InExtensionName)
    {
    }

public:
    virtual ~IVulkanExtension() = default;

    const char* GetExtensionName() const
    {
        return ExtensionName;
    }

    bool IsExtensionRequired() const
    {
        return bRequired;
    }

    void SetSupported(bool bInSupported)
    {
        bSupported = bInSupported;
    }

    bool IsSupported() const
    {
        return bSupported;
    }

private:
    bool bSupported = true;
    const bool bRequired = true;
    const char* const ExtensionName = nullptr;
};

/// Interface for Vulkan extensions that require device-level initialization
class IDeviceVulkanExtension : public IVulkanExtension
{
public:
    IDeviceVulkanExtension(const char* InExtensionName, bool bInRequired)
        : IVulkanExtension(InExtensionName, bInRequired)
    {
    }
    virtual ~IDeviceVulkanExtension() = default;

    virtual void PreDeviceCreated(VkDeviceCreateInfo& Info)
    {
        (void)Info;
    }

    virtual void PostDeviceCreated(FOptionalExtensionStatus&)
    {
    }
};

/// Interface for Vulkan extensions that require instance-level initialization
class IInstanceVulkanExtension : public IVulkanExtension
{
public:
    IInstanceVulkanExtension(const char* InExtensionName, bool bInRequired)
        : IVulkanExtension(InExtensionName, bInRequired)
    {
    }
    virtual ~IInstanceVulkanExtension() = default;

    virtual void PreInstanceCreated(VkInstanceCreateInfo&)
    {
    }
};

}    // namespace VulkanRHI
