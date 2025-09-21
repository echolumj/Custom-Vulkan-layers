#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "vk_layer_lmj_test.h"


VkResult MyVkLayer_CreateInstance(
    const VkInstanceCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkInstance* pInstance)
{
    printf("MyVulkanLayer: vkCreateInstance called!\n");

    // 1. Find and call vkCreateInstance of next layer
    VkLayerInstanceCreateInfo* chain_info = get_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);

    // 2. get GetInstanceProcAddr of next layer
    PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;

    // get vkCreateInstanceProcAddr of next layer
    PFN_vkCreateInstance fpCreateInstance = (PFN_vkCreateInstance)fpGetInstanceProcAddr(NULL, "vkCreateInstance");
    if (fpCreateInstance == NULL) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // 3. set up information for the next layer.
    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

    // 4. call vkCreateInstance of next layer
    VkResult result = fpCreateInstance(pCreateInfo, pAllocator, pInstance);
    if (result != VK_SUCCESS) {
        return result;
    }

    //gInstance = pInstance;
    // 5. create dispath table for current instance
    if(instance_dispatch.find(*pInstance) == instance_dispatch.end())
    {
        instance_dispatch[*pInstance] = VkLayerInstanceDispatchTable{};
        init_instance_dispatch_table(*pInstance, instance_dispatch[*pInstance],fpGetInstanceProcAddr);
    }

    printf("MyVulkanLayer: vkCreateInstance succeeded!\n");
    return VK_SUCCESS;
}


VkResult MyVkLayer_CreateDevice(
    VkPhysicalDevice physicalDevice,
    const VkDeviceCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDevice* pDevice)
{
    printf("MyVulkanLayer: vkCreateDevice called!\n");

    VkLayerDeviceCreateInfo *chain_info = get_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);
    PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    PFN_vkGetDeviceProcAddr fpGetDeviceProcAddr = chain_info->u.pLayerInfo->pfnNextGetDeviceProcAddr;

    PFN_vkCreateDevice fpCreateDevice = (PFN_vkCreateDevice)fpGetInstanceProcAddr(*gInstance, "vkCreateDevice");
    if (fpCreateDevice == NULL) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;
    VkResult ret = fpCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
    if (ret != VK_SUCCESS) {
        return ret;
    }

	//gDevice = pDevice;

    if(device_dispatch.find(*pDevice) == device_dispatch.end())
    {
        device_dispatch[*pDevice] = VkLayerDeviceDispatchTable{};
        init_device_dispatch_table(*pDevice, device_dispatch[*pDevice],fpGetDeviceProcAddr);
    }

    printf("MyVulkanLayer: vkCreateDevice succeeded!\n");
    return VK_SUCCESS;
}


PFN_vkVoidFunction MyVkLayer_GetDeviceProcAddr(VkDevice device, const char* pName) {

    // if name is NULL undefined is returned, let's return NULL
    if (pName == NULL)
        return NULL;

    // hook device-level functions even if device is NULL
    DeviceHook(GetDeviceProcAddr);
    DeviceHook(CreateDevice);


    if (device == VK_NULL_HANDLE) {
        return NULL;
    }

    //TODO: return hooked device-level functions if device is not NULL
    //TODO: update device dispatch table
    return device_dispatch[device].GetDeviceProcAddr(device, pName);
}


PFN_vkVoidFunction MyVkLayer_GetInstanceProcAddr(VkInstance instance, const char* pName) {
    
    // if name is NULL undefined is returned, let's return NULL
    if (pName == NULL)
        return NULL;

	// hook instance-level functions even if instance is NULL
    InstanceHook(GetInstanceProcAddr);
    InstanceHook(CreateInstance);

    if (instance == VK_NULL_HANDLE) {
        return NULL;
    }

	//hook instance-level functions when instance is not NULL
    InstanceHook(CreateDevice);
    InstanceHook(GetDeviceProcAddr);

    //TODO: update instance dispatch table
    return instance_dispatch[instance].GetInstanceProcAddr(instance, pName);
}

VkResult MyVkLayer_NegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface* pVersionStruct)
{
    assert(pVersionStruct);
    assert(pVersionStruct->sType == LAYER_NEGOTIATE_INTERFACE_STRUCT);
    assert(pVersionStruct->loaderLayerInterfaceVersion >= 2);

    pVersionStruct->pfnGetInstanceProcAddr = &MyVkLayer_GetInstanceProcAddr;
    pVersionStruct->pfnGetDeviceProcAddr = &MyVkLayer_GetDeviceProcAddr;

    return VK_SUCCESS;
}