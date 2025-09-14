#include <vulkan/vulkan.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// 定义分发表结构体
// 这里只包含两个函数作为示例，一个完整的 Layer 需要为所有它想拦截的函数定义指针
typedef struct VkLayerInstanceDispatchTable_ {
    PFN_vkGetInstanceProcAddr GetInstanceProcAddr;
    PFN_vkCreateInstance CreateInstance;
    // ... 其他 Instance 函数指针
} VkLayerInstanceDispatchTable;

typedef struct VkLayerDeviceDispatchTable_ {
    PFN_vkGetDeviceProcAddr GetDeviceProcAddr;
    PFN_vkCreateDevice CreateDevice;
    // ... 其他 Device 函数指针
} VkLayerDeviceDispatchTable;

// --- Instance 链的扩展处理 ---
VkResult MyVkLayer_CreateInstance(
    const VkInstanceCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkInstance* pInstance)
{
    printf("MyVulkanLayer: vkCreateInstance called!\n");

    // 1. 查找并调用下一层的 vkCreateInstance
    VkLayerInstanceCreateInfo* layerCreateInfo = (VkLayerInstanceCreateInfo*)pCreateInfo->pNext;

    // 遍历 pNext 链，找到 VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO
    while(layerCreateInfo && !(layerCreateInfo->sType == VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO && layerCreateInfo->function == VK_LAYER_LINK_INFO)) {
        layerCreateInfo = (VkLayerInstanceCreateInfo*)layerCreateInfo->pNext;
    }

    if (layerCreateInfo == NULL) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // 2. 获取下一层的 GetInstanceProcAddr 函数
    PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr = layerCreateInfo->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    // 获取下一层的 vkCreateInstance 函数
    PFN_vkCreateInstance fpCreateInstance = (PFN_vkCreateInstance)fpGetInstanceProcAddr(NULL, "vkCreateInstance");
    if (fpCreateInstance == NULL) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // 3. 移动链指针，为下一层设置信息
    layerCreateInfo->u.pLayerInfo = layerCreateInfo->u.pLayerInfo->pNext;

    // 4. 调用下一层的 vkCreateInstance
    VkResult result = fpCreateInstance(pCreateInfo, pAllocator, pInstance);
    if (result != VK_SUCCESS) {
        return result;
    }

    // 5. 为新建的 Instance 创建并初始化我们的分发表
    // 这里我们简化处理，只是演示
    // 实际你需要为这个 *pInstance 关联一个分发表，并通过 fpGetInstanceProcAddr 填充它
    // 例如：创建一个 dispatch table，并填充 fpGetInstanceProcAddr, fpCreateDevice 等函数指针

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

    // 类似于 CreateInstance 的逻辑，需要链接到下一层
    // ... (代码省略，逻辑同 CreateInstance)

    // 假设我们获取到了下一层的 fpCreateDevice
    PFN_vkCreateDevice fpCreateDevice = ...; // 通过物理设备的分发表获取

    VkResult result = fpCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
    if (result != VK_SUCCESS) {
        return result;
    }

    // 初始化设备的分发表
    // ...

    printf("MyVulkanLayer: vkCreateDevice succeeded!\n");
    return VK_SUCCESS;
}

// --- 最核心的函数：获取实例过程地址 ---
// Vulkan 加载器首先调用这个函数来获取我们 Layer 中实现的函数指针
PFN_vkVoidFunction MyVkLayer_GetInstanceProcAddr(VkInstance instance, const char* pName) {
    // 拦截我们想要处理的函数
    if (strcmp(pName, "vkCreateInstance") == 0) {
        return (PFN_vkVoidFunction)MyVkLayer_CreateInstance;
    }
    if (strcmp(pName, "vkCreateDevice") == 0) {
        return (PFN_vkVoidFunction)MyVkLayer_CreateDevice;
    }
    if (strcmp(pName, "vkGetInstanceProcAddr") == 0) {
        return (PFN_vkVoidFunction)MyVkLayer_GetInstanceProcAddr;
    }
    if (strcmp(pName, "vkGetDeviceProcAddr") == 0) {
        return (PFN_vkVoidFunction)MyVkLayer_GetDeviceProcAddr;
    }

    // 对于其他函数，如果 instance 不为 NULL，我们应该从下一层的分发表中获取
    if (instance != NULL) {
        // 这里需要从与这个 instance 关联的我们的分发表中获取下一层的 GetInstanceProcAddr
        // VkLayerInstanceDispatchTable* dispatchTable = ...(从 instance 的映射中获取)...
        // return dispatchTable->GetInstanceProcAddr(instance, pName);
        // 为了示例简单，我们返回 NULL
    }

    return NULL;
}

// --- 获取设备过程地址 ---
PFN_vkVoidFunction MyVkLayer_GetDeviceProcAddr(VkDevice device, const char* pName) {
    // 拦截设备级别的函数，例如 vkCmdDraw*
    if (strcmp(pName, "vkCmdDraw") == 0) {
        // return (PFN_vkVoidFunction)MyVkLayer_CmdDraw;
    }

    // 对于其他函数，从下一层的设备分发表中获取
    // VkLayerDeviceDispatchTable* dispatchTable = ...(从 device 的映射中获取)...
    // return dispatchTable->GetDeviceProcAddr(device, pName);

    return NULL;
}