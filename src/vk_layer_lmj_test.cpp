#include "vk_layer_func.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>


VkResult MyVkLayer_NegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface *pVersionStruct)
{
    assert(pVersionStruct);
    assert(pVersionStruct->sType == LAYER_NEGOTIATE_INTERFACE_STRUCT);

    assert(pVersionStruct->loaderLayerInterfaceVersion >= 2);

    /* Fill in struct values. */
   pVersionStruct->pfnGetInstanceProcAddr = &MyVkLayer_GetInstanceProcAddr;
   pVersionStruct->pfnGetDeviceProcAddr = &MyVkLayer_GetDeviceProcAddr;

    return VK_SUCCESS;
}

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
        instance_dispatch[*pInstance] = new VkLayerInstanceDispatchTable;
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

     // 1. 查找并调用下一层的 vkCreateInstance
    VkLayerDeviceCreateInfo *chain_info = get_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);

    // 2. 获取下一层的 GetInstanceProcAddr 函数
    PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    PFN_vkGetDeviceProcAddr fpGetDeviceProcAddr = chain_info->u.pLayerInfo->pfnNextGetDeviceProcAddr;

    // 获取下一层的 vkCreateInstance 函数
    PFN_vkCreateDevice fpCreateDevice = (PFN_vkCreateDevice)fpGetInstanceProcAddr(*gInstance, "vkCreateDevice");
    if (fpCreateDevice == NULL) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // 3. 移动链指针，为下一层设置信息
    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

    // 4. 调用下一层的 vkCreateInstance
    VkResult ret = fpCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
    if (ret != VK_SUCCESS) {
        return ret;
    }

	gDevice = pDevice;

    // 初始化设备的分发表
    if(device_dispatch.find(*pDevice) == device_dispatch.end())
    {
        device_dispatch[*pDevice] = new VkLayerDeviceDispatchTable;
        init_device_dispatch_table(*pDevice, device_dispatch[*pDevice],fpGetDeviceProcAddr);
    }

    printf("MyVulkanLayer: vkCreateDevice succeeded!\n");
    return VK_SUCCESS;
}

// --- 最核心的函数：获取实例过程地址 ---
// Vulkan 加载器首先调用这个函数来获取我们 Layer 中实现的函数指针
PFN_vkVoidFunction MyVkLayer_GetInstanceProcAddr(VkInstance instance, const char* pName) {
    
    // 拦截我们想要处理的函数
	//InstanceHook(DestroyInstance);

    // 对于其他函数，如果 instance 不为 NULL，我们应该从下一层的分发表中获取
    if (instance != NULL) {
        return instance_dispatch[instance]->GetInstanceProcAddr(instance, pName);
    }

    return NULL;
}

// --- 获取设备过程地址 ---
PFN_vkVoidFunction MyVkLayer_GetDeviceProcAddr(VkDevice device, const char* pName) {
    
    // 拦截设备级别的函数
    //DeviceHook(DestroyDevice);

    // 对于其他函数，从下一层的设备分发表中获取
    if (device != NULL) {
        return device_dispatch[device]->GetDeviceProcAddr(device, pName);
    }

    return NULL;
}
