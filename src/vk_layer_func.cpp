#include "vk_layer_func.h"
#include <assert.h>

std::unordered_map<VkInstance, VkLayerInstanceDispatchTable*> instance_dispatch;
std::unordered_map<VkDevice, VkLayerDeviceDispatchTable*> device_dispatch;

VkInstance* gInstance = nullptr;
VkDevice* gDevice = nullptr;

VkLayerInstanceCreateInfo *get_chain_info(const VkInstanceCreateInfo *pCreateInfo, VkLayerFunction func) {
    VkLayerInstanceCreateInfo *chain_info = (VkLayerInstanceCreateInfo *)pCreateInfo->pNext;
    while (chain_info && !(chain_info->sType == VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO && chain_info->function == func)) {
        chain_info = (VkLayerInstanceCreateInfo *)chain_info->pNext;
    }
    assert(chain_info != NULL);
    return chain_info;
}

VkLayerDeviceCreateInfo *get_chain_info(const VkDeviceCreateInfo *pCreateInfo, VkLayerFunction func) {
    VkLayerDeviceCreateInfo *chain_info = (VkLayerDeviceCreateInfo *)pCreateInfo->pNext;
    while (chain_info && !(chain_info->sType == VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO && chain_info->function == func)) {
        chain_info = (VkLayerDeviceCreateInfo *)chain_info->pNext;
    }
    assert(chain_info != NULL);
    return chain_info;
}

void init_device_dispatch_table(VkDevice device, VkLayerDeviceDispatchTable* table, PFN_vkGetDeviceProcAddr addr)
{
    memset(table, 0, sizeof(*table));
    table->GetDeviceProcAddr = addr;
    table->DestoryDevice = (PFN_vkDestroyDevice)addr(device, "vkDestroyDevice");
}

void init_instance_dispatch_table(VkInstance instance, VkLayerInstanceDispatchTable* table, PFN_vkGetInstanceProcAddr addr)
{
    memset(table, 0, sizeof(*table));
    table->GetInstanceProcAddr = addr;
    table->DestroyInstance = (PFN_vkDestroyInstance)addr(instance, "vkDestroyInstance");
}