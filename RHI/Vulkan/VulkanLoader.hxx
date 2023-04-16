#pragma once

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#define VK_ENTRYPOINTS_INSTANCE(LoadMacro)                                                                         \
    LoadMacro(PFN_vkDestroyInstance, vkDestroyInstance);                                                           \
    LoadMacro(PFN_vkEnumeratePhysicalDevices, vkEnumeratePhysicalDevices);                                         \
    LoadMacro(PFN_vkGetPhysicalDeviceFeatures, vkGetPhysicalDeviceFeatures);                                       \
    LoadMacro(PFN_vkGetPhysicalDeviceFormatProperties, vkGetPhysicalDeviceFormatProperties);                       \
    LoadMacro(PFN_vkGetPhysicalDeviceImageFormatProperties, vkGetPhysicalDeviceImageFormatProperties);             \
    LoadMacro(PFN_vkGetPhysicalDeviceProperties, vkGetPhysicalDeviceProperties);                                   \
    LoadMacro(PFN_vkGetPhysicalDeviceQueueFamilyProperties, vkGetPhysicalDeviceQueueFamilyProperties);             \
    LoadMacro(PFN_vkGetPhysicalDeviceMemoryProperties, vkGetPhysicalDeviceMemoryProperties);                       \
    LoadMacro(PFN_vkCreateDevice, vkCreateDevice);                                                                 \
    LoadMacro(PFN_vkDestroyDevice, vkDestroyDevice);                                                               \
    LoadMacro(PFN_vkEnumerateDeviceExtensionProperties, vkEnumerateDeviceExtensionProperties);                     \
    LoadMacro(PFN_vkEnumerateDeviceLayerProperties, vkEnumerateDeviceLayerProperties);                             \
    LoadMacro(PFN_vkGetDeviceQueue, vkGetDeviceQueue);                                                             \
    LoadMacro(PFN_vkQueueSubmit, vkQueueSubmit);                                                                   \
    LoadMacro(PFN_vkQueueWaitIdle, vkQueueWaitIdle);                                                               \
    LoadMacro(PFN_vkDeviceWaitIdle, vkDeviceWaitIdle);                                                             \
    LoadMacro(PFN_vkAllocateMemory, vkAllocateMemory);                                                             \
    LoadMacro(PFN_vkFreeMemory, vkFreeMemory);                                                                     \
    LoadMacro(PFN_vkMapMemory, vkMapMemory);                                                                       \
    LoadMacro(PFN_vkUnmapMemory, vkUnmapMemory);                                                                   \
    LoadMacro(PFN_vkFlushMappedMemoryRanges, vkFlushMappedMemoryRanges);                                           \
    LoadMacro(PFN_vkInvalidateMappedMemoryRanges, vkInvalidateMappedMemoryRanges);                                 \
    LoadMacro(PFN_vkGetDeviceMemoryCommitment, vkGetDeviceMemoryCommitment);                                       \
    LoadMacro(PFN_vkBindBufferMemory, vkBindBufferMemory);                                                         \
    LoadMacro(PFN_vkBindImageMemory, vkBindImageMemory);                                                           \
    LoadMacro(PFN_vkGetBufferMemoryRequirements, vkGetBufferMemoryRequirements);                                   \
    LoadMacro(PFN_vkGetImageMemoryRequirements, vkGetImageMemoryRequirements);                                     \
    LoadMacro(PFN_vkGetImageSparseMemoryRequirements, vkGetImageSparseMemoryRequirements);                         \
    LoadMacro(PFN_vkGetPhysicalDeviceSparseImageFormatProperties, vkGetPhysicalDeviceSparseImageFormatProperties); \
    LoadMacro(PFN_vkQueueBindSparse, vkQueueBindSparse);                                                           \
    LoadMacro(PFN_vkCreateFence, vkCreateFence);                                                                   \
    LoadMacro(PFN_vkDestroyFence, vkDestroyFence);                                                                 \
    LoadMacro(PFN_vkResetFences, vkResetFences);                                                                   \
    LoadMacro(PFN_vkGetFenceStatus, vkGetFenceStatus);                                                             \
    LoadMacro(PFN_vkWaitForFences, vkWaitForFences);                                                               \
    LoadMacro(PFN_vkCreateSemaphore, vkCreateSemaphore);                                                           \
    LoadMacro(PFN_vkDestroySemaphore, vkDestroySemaphore);                                                         \
    LoadMacro(PFN_vkCreateEvent, vkCreateEvent);                                                                   \
    LoadMacro(PFN_vkDestroyEvent, vkDestroyEvent);                                                                 \
    LoadMacro(PFN_vkGetEventStatus, vkGetEventStatus);                                                             \
    LoadMacro(PFN_vkSetEvent, vkSetEvent);                                                                         \
    LoadMacro(PFN_vkResetEvent, vkResetEvent);                                                                     \
    LoadMacro(PFN_vkCreateQueryPool, vkCreateQueryPool);                                                           \
    LoadMacro(PFN_vkDestroyQueryPool, vkDestroyQueryPool);                                                         \
    LoadMacro(PFN_vkGetQueryPoolResults, vkGetQueryPoolResults);                                                   \
    LoadMacro(PFN_vkCreateBuffer, vkCreateBuffer);                                                                 \
    LoadMacro(PFN_vkDestroyBuffer, vkDestroyBuffer);                                                               \
    LoadMacro(PFN_vkCreateBufferView, vkCreateBufferView);                                                         \
    LoadMacro(PFN_vkDestroyBufferView, vkDestroyBufferView);                                                       \
    LoadMacro(PFN_vkCreateImage, vkCreateImage);                                                                   \
    LoadMacro(PFN_vkDestroyImage, vkDestroyImage);                                                                 \
    LoadMacro(PFN_vkGetImageSubresourceLayout, vkGetImageSubresourceLayout);                                       \
    LoadMacro(PFN_vkCreateImageView, vkCreateImageView);                                                           \
    LoadMacro(PFN_vkDestroyImageView, vkDestroyImageView);                                                         \
    LoadMacro(PFN_vkCreateShaderModule, vkCreateShaderModule);                                                     \
    LoadMacro(PFN_vkDestroyShaderModule, vkDestroyShaderModule);                                                   \
    LoadMacro(PFN_vkCreatePipelineCache, vkCreatePipelineCache);                                                   \
    LoadMacro(PFN_vkDestroyPipelineCache, vkDestroyPipelineCache);                                                 \
    LoadMacro(PFN_vkGetPipelineCacheData, vkGetPipelineCacheData);                                                 \
    LoadMacro(PFN_vkMergePipelineCaches, vkMergePipelineCaches);                                                   \
    LoadMacro(PFN_vkCreateGraphicsPipelines, vkCreateGraphicsPipelines);                                           \
    LoadMacro(PFN_vkCreateComputePipelines, vkCreateComputePipelines);                                             \
    LoadMacro(PFN_vkDestroyPipeline, vkDestroyPipeline);                                                           \
    LoadMacro(PFN_vkCreatePipelineLayout, vkCreatePipelineLayout);                                                 \
    LoadMacro(PFN_vkDestroyPipelineLayout, vkDestroyPipelineLayout);                                               \
    LoadMacro(PFN_vkCreateSampler, vkCreateSampler);                                                               \
    LoadMacro(PFN_vkDestroySampler, vkDestroySampler);                                                             \
    LoadMacro(PFN_vkCreateDescriptorSetLayout, vkCreateDescriptorSetLayout);                                       \
    LoadMacro(PFN_vkDestroyDescriptorSetLayout, vkDestroyDescriptorSetLayout);                                     \
    LoadMacro(PFN_vkCreateDescriptorPool, vkCreateDescriptorPool);                                                 \
    LoadMacro(PFN_vkDestroyDescriptorPool, vkDestroyDescriptorPool);                                               \
    LoadMacro(PFN_vkResetDescriptorPool, vkResetDescriptorPool);                                                   \
    LoadMacro(PFN_vkAllocateDescriptorSets, vkAllocateDescriptorSets);                                             \
    LoadMacro(PFN_vkFreeDescriptorSets, vkFreeDescriptorSets);                                                     \
    LoadMacro(PFN_vkUpdateDescriptorSets, vkUpdateDescriptorSets);                                                 \
    LoadMacro(PFN_vkCreateFramebuffer, vkCreateFramebuffer);                                                       \
    LoadMacro(PFN_vkDestroyFramebuffer, vkDestroyFramebuffer);                                                     \
    LoadMacro(PFN_vkCreateRenderPass, vkCreateRenderPass);                                                         \
    LoadMacro(PFN_vkDestroyRenderPass, vkDestroyRenderPass);                                                       \
    LoadMacro(PFN_vkGetRenderAreaGranularity, vkGetRenderAreaGranularity);                                         \
    LoadMacro(PFN_vkCreateCommandPool, vkCreateCommandPool);                                                       \
    LoadMacro(PFN_vkDestroyCommandPool, vkDestroyCommandPool);                                                     \
    LoadMacro(PFN_vkResetCommandPool, vkResetCommandPool);                                                         \
    LoadMacro(PFN_vkAllocateCommandBuffers, vkAllocateCommandBuffers);                                             \
    LoadMacro(PFN_vkFreeCommandBuffers, vkFreeCommandBuffers);                                                     \
    LoadMacro(PFN_vkBeginCommandBuffer, vkBeginCommandBuffer);                                                     \
    LoadMacro(PFN_vkEndCommandBuffer, vkEndCommandBuffer);                                                         \
    LoadMacro(PFN_vkResetCommandBuffer, vkResetCommandBuffer);                                                     \
    LoadMacro(PFN_vkCmdBindPipeline, vkCmdBindPipeline);                                                           \
    LoadMacro(PFN_vkCmdSetViewport, vkCmdSetViewport);                                                             \
    LoadMacro(PFN_vkCmdSetScissor, vkCmdSetScissor);                                                               \
    LoadMacro(PFN_vkCmdSetLineWidth, vkCmdSetLineWidth);                                                           \
    LoadMacro(PFN_vkCmdSetDepthBias, vkCmdSetDepthBias);                                                           \
    LoadMacro(PFN_vkCmdSetBlendConstants, vkCmdSetBlendConstants);                                                 \
    LoadMacro(PFN_vkCmdSetDepthBounds, vkCmdSetDepthBounds);                                                       \
    LoadMacro(PFN_vkCmdSetStencilCompareMask, vkCmdSetStencilCompareMask);                                         \
    LoadMacro(PFN_vkCmdSetStencilWriteMask, vkCmdSetStencilWriteMask);                                             \
    LoadMacro(PFN_vkCmdSetStencilReference, vkCmdSetStencilReference);                                             \
    LoadMacro(PFN_vkCmdBindDescriptorSets, vkCmdBindDescriptorSets);                                               \
    LoadMacro(PFN_vkCmdBindIndexBuffer, vkCmdBindIndexBuffer);                                                     \
    LoadMacro(PFN_vkCmdBindVertexBuffers, vkCmdBindVertexBuffers);                                                 \
    LoadMacro(PFN_vkCmdDraw, vkCmdDraw);                                                                           \
    LoadMacro(PFN_vkCmdDrawIndexed, vkCmdDrawIndexed);                                                             \
    LoadMacro(PFN_vkCmdDrawIndirect, vkCmdDrawIndirect);                                                           \
    LoadMacro(PFN_vkCmdDrawIndexedIndirect, vkCmdDrawIndexedIndirect);                                             \
    LoadMacro(PFN_vkCmdDispatch, vkCmdDispatch);                                                                   \
    LoadMacro(PFN_vkCmdDispatchIndirect, vkCmdDispatchIndirect);                                                   \
    LoadMacro(PFN_vkCmdCopyBuffer, vkCmdCopyBuffer);                                                               \
    LoadMacro(PFN_vkCmdCopyImage, vkCmdCopyImage);                                                                 \
    LoadMacro(PFN_vkCmdBlitImage, vkCmdBlitImage);                                                                 \
    LoadMacro(PFN_vkCmdCopyBufferToImage, vkCmdCopyBufferToImage);                                                 \
    LoadMacro(PFN_vkCmdCopyImageToBuffer, vkCmdCopyImageToBuffer);                                                 \
    LoadMacro(PFN_vkCmdUpdateBuffer, vkCmdUpdateBuffer);                                                           \
    LoadMacro(PFN_vkCmdFillBuffer, vkCmdFillBuffer);                                                               \
    LoadMacro(PFN_vkCmdClearColorImage, vkCmdClearColorImage);                                                     \
    LoadMacro(PFN_vkCmdClearDepthStencilImage, vkCmdClearDepthStencilImage);                                       \
    LoadMacro(PFN_vkCmdClearAttachments, vkCmdClearAttachments);                                                   \
    LoadMacro(PFN_vkCmdResolveImage, vkCmdResolveImage);                                                           \
    LoadMacro(PFN_vkCmdSetEvent, vkCmdSetEvent);                                                                   \
    LoadMacro(PFN_vkCmdResetEvent, vkCmdResetEvent);                                                               \
    LoadMacro(PFN_vkCmdWaitEvents, vkCmdWaitEvents);                                                               \
    LoadMacro(PFN_vkCmdPipelineBarrier, vkCmdPipelineBarrier);                                                     \
    LoadMacro(PFN_vkCmdBeginQuery, vkCmdBeginQuery);                                                               \
    LoadMacro(PFN_vkCmdEndQuery, vkCmdEndQuery);                                                                   \
    LoadMacro(PFN_vkCmdResetQueryPool, vkCmdResetQueryPool);                                                       \
    LoadMacro(PFN_vkCmdWriteTimestamp, vkCmdWriteTimestamp);                                                       \
    LoadMacro(PFN_vkCmdCopyQueryPoolResults, vkCmdCopyQueryPoolResults);                                           \
    LoadMacro(PFN_vkCmdPushConstants, vkCmdPushConstants);                                                         \
    LoadMacro(PFN_vkCmdBeginRenderPass, vkCmdBeginRenderPass);                                                     \
    LoadMacro(PFN_vkCmdNextSubpass, vkCmdNextSubpass);                                                             \
    LoadMacro(PFN_vkCmdEndRenderPass, vkCmdEndRenderPass);                                                         \
    LoadMacro(PFN_vkCmdExecuteCommands, vkCmdExecuteCommands);                                                     \
    LoadMacro(PFN_vkCreateSwapchainKHR, vkCreateSwapchainKHR);                                                     \
    LoadMacro(PFN_vkDestroySwapchainKHR, vkDestroySwapchainKHR);                                                   \
    LoadMacro(PFN_vkGetSwapchainImagesKHR, vkGetSwapchainImagesKHR);                                               \
    LoadMacro(PFN_vkAcquireNextImageKHR, vkAcquireNextImageKHR);                                                   \
    LoadMacro(PFN_vkQueuePresentKHR, vkQueuePresentKHR);

#define VK_ENTRYPOINTS_SURFACE_INSTANCE(LoadMacro)                                                       \
    LoadMacro(PFN_vkDestroySurfaceKHR, vkDestroySurfaceKHR);                                             \
    LoadMacro(PFN_vkGetPhysicalDeviceSurfaceSupportKHR, vkGetPhysicalDeviceSurfaceSupportKHR);           \
    LoadMacro(PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR, vkGetPhysicalDeviceSurfaceCapabilitiesKHR); \
    LoadMacro(PFN_vkGetPhysicalDeviceSurfaceFormatsKHR, vkGetPhysicalDeviceSurfaceFormatsKHR);           \
    LoadMacro(PFN_vkGetPhysicalDeviceSurfacePresentModesKHR, vkGetPhysicalDeviceSurfacePresentModesKHR);

#define VK_ENTRYPOINTS_BASE(LoadMacro)                                                             \
    LoadMacro(PFN_vkCreateInstance, vkCreateInstance);                                             \
    LoadMacro(PFN_vkGetInstanceProcAddr, vkGetInstanceProcAddr);                                   \
    LoadMacro(PFN_vkGetDeviceProcAddr, vkGetDeviceProcAddr);                                       \
    LoadMacro(PFN_vkEnumerateInstanceExtensionProperties, vkEnumerateInstanceExtensionProperties); \
    LoadMacro(PFN_vkEnumerateInstanceLayerProperties, vkEnumerateInstanceLayerProperties);

#define VK_ENTRYPOINTS_OPTIONAL_BASE(LoadMacro)                                                                \
    LoadMacro(PFN_vkGetPhysicalDeviceDisplayPropertiesKHR, vkGetPhysicalDeviceDisplayPropertiesKHR);           \
    LoadMacro(PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR, vkGetPhysicalDeviceDisplayPlanePropertiesKHR); \
    LoadMacro(PFN_vkGetDisplayPlaneSupportedDisplaysKHR, vkGetDisplayPlaneSupportedDisplaysKHR);               \
    LoadMacro(PFN_vkGetDisplayModePropertiesKHR, vkGetDisplayModePropertiesKHR);                               \
    LoadMacro(PFN_vkCreateDisplayModeKHR, vkCreateDisplayModeKHR);                                             \
    LoadMacro(PFN_vkGetDisplayPlaneCapabilitiesKHR, vkGetDisplayPlaneCapabilitiesKHR);

#define VK_ENTRYPOINTS_DEBUG_UTILS(LoadMacro)                                        \
    LoadMacro(PFN_vkCmdBeginDebugUtilsLabelEXT, vkCmdBeginDebugUtilsLabelEXT);       \
    LoadMacro(PFN_vkCmdEndDebugUtilsLabelEXT, vkCmdEndDebugUtilsLabelEXT);           \
    LoadMacro(PFN_vkCmdInsertDebugUtilsLabelEXT, vkCmdInsertDebugUtilsLabelEXT);     \
    LoadMacro(PFN_vkCreateDebugUtilsMessengerEXT, vkCreateDebugUtilsMessengerEXT);   \
    LoadMacro(PFN_vkDestroyDebugUtilsMessengerEXT, vkDestroyDebugUtilsMessengerEXT); \
    LoadMacro(PFN_vkQueueBeginDebugUtilsLabelEXT, vkQueueBeginDebugUtilsLabelEXT);   \
    LoadMacro(PFN_vkQueueEndDebugUtilsLabelEXT, vkQueueEndDebugUtilsLabelEXT);       \
    LoadMacro(PFN_vkQueueInsertDebugUtilsLabelEXT, vkQueueInsertDebugUtilsLabelEXT); \
    LoadMacro(PFN_vkSetDebugUtilsObjectNameEXT, vkSetDebugUtilsObjectNameEXT);       \
    LoadMacro(PFN_vkSetDebugUtilsObjectTagEXT, vkSetDebugUtilsObjectTagEXT);         \
    LoadMacro(PFN_vkSubmitDebugUtilsMessageEXT, vkSubmitDebugUtilsMessageEXT);

#define VK_ENTRYPOINT_ALL(LoadMacro)            \
    VK_ENTRYPOINTS_INSTANCE(LoadMacro);         \
    VK_ENTRYPOINTS_SURFACE_INSTANCE(LoadMacro); \
    VK_ENTRYPOINTS_BASE(LoadMacro);             \
    VK_ENTRYPOINTS_OPTIONAL_BASE(LoadMacro);

// Get Vulkan function to vma
#define VK_ENTRYPOINTS_VMA(LoadMacro)                                                        \
    LoadMacro(PFN_vkGetInstanceProcAddr, vkGetInstanceProcAddr);                             \
    LoadMacro(PFN_vkGetDeviceProcAddr, vkGetDeviceProcAddr);                                 \
    LoadMacro(PFN_vkGetPhysicalDeviceProperties, vkGetPhysicalDeviceProperties);             \
    LoadMacro(PFN_vkGetPhysicalDeviceMemoryProperties, vkGetPhysicalDeviceMemoryProperties); \
    LoadMacro(PFN_vkAllocateMemory, vkAllocateMemory);                                       \
    LoadMacro(PFN_vkFreeMemory, vkFreeMemory);                                               \
    LoadMacro(PFN_vkMapMemory, vkMapMemory);                                                 \
    LoadMacro(PFN_vkUnmapMemory, vkUnmapMemory);                                             \
    LoadMacro(PFN_vkFlushMappedMemoryRanges, vkFlushMappedMemoryRanges);                     \
    LoadMacro(PFN_vkInvalidateMappedMemoryRanges, vkInvalidateMappedMemoryRanges);           \
    LoadMacro(PFN_vkBindBufferMemory, vkBindBufferMemory);                                   \
    LoadMacro(PFN_vkBindImageMemory, vkBindImageMemory);                                     \
    LoadMacro(PFN_vkGetBufferMemoryRequirements, vkGetBufferMemoryRequirements);             \
    LoadMacro(PFN_vkGetImageMemoryRequirements, vkGetImageMemoryRequirements);               \
    LoadMacro(PFN_vkCreateBuffer, vkCreateBuffer);                                           \
    LoadMacro(PFN_vkDestroyBuffer, vkDestroyBuffer);                                         \
    LoadMacro(PFN_vkCreateImage, vkCreateImage);                                             \
    LoadMacro(PFN_vkDestroyImage, vkDestroyImage);                                           \
    LoadMacro(PFN_vkCmdCopyBuffer, vkCmdCopyBuffer);                                         \
    LoadMacro(PFN_vkGetBufferMemoryRequirements2KHR, vkGetBufferMemoryRequirements2KHR);     \
    LoadMacro(PFN_vkGetImageMemoryRequirements2KHR, vkGetImageMemoryRequirements2KHR);       \
    LoadMacro(PFN_vkBindBufferMemory2KHR, vkBindBufferMemory2KHR);                           \
    LoadMacro(PFN_vkBindImageMemory2KHR, vkBindImageMemory2KHR);                             \
    LoadMacro(PFN_vkGetPhysicalDeviceMemoryProperties2KHR, vkGetPhysicalDeviceMemoryProperties2KHR);

#define DECLARE_VK_ENTRYPOINTS(Type, Func) extern Type Func;

namespace VulkanRHI
{

namespace VulkanAPI
{

    VK_ENTRYPOINT_ALL(DECLARE_VK_ENTRYPOINTS);
    VK_ENTRYPOINTS_DEBUG_UTILS(DECLARE_VK_ENTRYPOINTS);

}    // namespace VulkanAPI

}    // namespace VulkanRHI
