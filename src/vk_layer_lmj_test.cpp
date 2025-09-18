#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "vk_layer_lmj_test.h"

// --- Instance 链的扩展处理 ---
VkResult MyVkLayer_CreateInstance(
    const VkInstanceCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkInstance* pInstance)
{
    printf("MyVulkanLayer: vkCreateInstance called!\n");

    // 1. 查找并调用下一层的 vkCreateInstance
    VkLayerInstanceCreateInfo* chain_info = get_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);

    // 2. 获取下一层的 GetInstanceProcAddr 函数
    PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;

    // 获取下一层的 vkCreateInstance 函数
    PFN_vkCreateInstance fpCreateInstance = (PFN_vkCreateInstance)fpGetInstanceProcAddr(NULL, "vkCreateInstance");
    if (fpCreateInstance == NULL) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // 3. 移动链指针，为下一层设置信息
    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

    // 4. 调用下一层的 vkCreateInstance
    VkResult result = fpCreateInstance(pCreateInfo, pAllocator, pInstance);
    if (result != VK_SUCCESS) {
        return result;
    }

    gInstance = pInstance;
    // 5. 为新建的 Instance 创建并初始化我们的分发表
    if(instance_dispatch.find(*pInstance) == instance_dispatch.end())
    {
        instance_dispatch[*pInstance] = VkLayerInstanceDispatchTable{};
        init_instance_dispatch_table(*pInstance, instance_dispatch[*pInstance],fpGetInstanceProcAddr);
    }

    printf("MyVulkanLayer: vkCreateInstance succeeded!\n");
    return VK_SUCCESS;
}

// --- 类似的 Device 函数 ---
VkResult MyVkLayer_CreateDevice(
    VkPhysicalDevice physicalDevice,
    const VkDeviceCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDevice* pDevice)
{
    printf("MyVulkanLayer: vkCreateDevice called!\n");

     // 1. 查找并调用下一层的 vkCreateDevice
    VkLayerDeviceCreateInfo *chain_info = get_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);

    // 2. 获取下一层的 GetDeviceProcAddr 函数
    PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    PFN_vkGetDeviceProcAddr fpGetDeviceProcAddr = chain_info->u.pLayerInfo->pfnNextGetDeviceProcAddr;

    // 获取下一层的 vkCreateDevice 函数
    PFN_vkCreateDevice fpCreateDevice = (PFN_vkCreateDevice)fpGetInstanceProcAddr(*gInstance, "vkCreateDevice");
    if (fpCreateDevice == NULL) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // 3. 移动链指针，为下一层设置信息
    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

    // 4. 调用下一层的 vkCreateDevice
    VkResult ret = fpCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
    if (ret != VK_SUCCESS) {
        return ret;
    }

	gDevice = pDevice;

    // 初始化设备的分发表
    if(device_dispatch.find(*pDevice) == device_dispatch.end())
    {
        device_dispatch[*pDevice] = VkLayerDeviceDispatchTable{};
        init_device_dispatch_table(*pDevice, device_dispatch[*pDevice],fpGetDeviceProcAddr);
    }

    printf("MyVulkanLayer: vkCreateDevice succeeded!\n");
    return VK_SUCCESS;
}

// --- 获取设备过程地址 ---
PFN_vkVoidFunction MyVkLayer_GetDeviceProcAddr(VkDevice device, const char* pName) {

    // if name is NULL undefined is returned, let's return NULL
    if (pName == NULL)
        return NULL;

    // hook device-level functions even if device is NULL
    DeviceHook(GetDeviceProcAddr);
    DeviceHook(CreateDevice);

    //TODO: return not hooked instance-level functions if device is NULL

    // 对于其他函数，从下一层的设备分发表中获取
    if (device != VK_NULL_HANDLE) {
        return NULL;
    }

    //TODO: return hooked instance-level functions if device is not NULL
    // 
    //TODO: update device dispatch table
    return device_dispatch[device].GetDeviceProcAddr(device, pName);
}

// --- 最核心的函数：获取实例过程地址 ---
// Vulkan 加载器首先调用这个函数来获取我们 Layer 中实现的函数指针
PFN_vkVoidFunction MyVkLayer_GetInstanceProcAddr(VkInstance instance, const char* pName) {
    
    // if name is NULL undefined is returned, let's return NULL
    if (pName == NULL)
        return NULL;

	// hook instance-level functions even if instance is NULL
    InstanceHook(GetInstanceProcAddr);
    InstanceHook(CreateInstance);

    // 对于其他函数，如果 instance 不为 NULL，我们应该从下一层的分发表中获取
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

    /* Fill in struct values. */
    pVersionStruct->pfnGetInstanceProcAddr = &MyVkLayer_GetInstanceProcAddr;
    pVersionStruct->pfnGetDeviceProcAddr = &MyVkLayer_GetDeviceProcAddr;

    return VK_SUCCESS;
}