#pragma once
#include <iostream>
#include <vulkan/vulkan.h>
#include "vk_layer_func.h"

extern "C" VK_API_EXPORT  VkResult MyVkLayer_CreateInstance(
    const VkInstanceCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkInstance* pInstance);

extern "C" VK_API_EXPORT VkResult MyVkLayer_CreateDevice(
    VkPhysicalDevice physicalDevice,
    const VkDeviceCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDevice* pDevice);

extern "C" VK_API_EXPORT PFN_vkVoidFunction MyVkLayer_GetDeviceProcAddr(VkDevice device, const char* pName);
extern "C" VK_API_EXPORT PFN_vkVoidFunction MyVkLayer_GetInstanceProcAddr(VkInstance instance, const char* pName);
extern "C" VkResult MyVkLayer_NegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface* pVersionStruct);
