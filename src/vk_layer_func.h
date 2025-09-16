#include <vulkan/vulkan.h>
#include <vulkan/vk_layer.h>
#include <iostream>
#include <unordered_map>

#define InstanceHook(func) \
	(void)((VkLayerInstanceDispatchTable*)0)->func; \
	if(!strcmp(pName, "vk" #func)) return (PFN_vkVoidFunction)&MyVkLayer_##func;
#define DeviceHook(func) \
	(void)((VkLayerDeviceDispatchTable*)0)->func; \
	if(!strcmp(pName, "vk" #func)) return (PFN_vkVoidFunction)&MyVkLayer_##func;

typedef struct VkLayerInstanceDispatchTable_ {
    PFN_vkGetInstanceProcAddr GetInstanceProcAddr;
    PFN_vkDestroyInstance DestroyInstance;
    // ... 其他 Instance 函数指针
} VkLayerInstanceDispatchTable;

typedef struct VkLayerDeviceDispatchTable_ {
    PFN_vkGetDeviceProcAddr GetDeviceProcAddr;
    PFN_vkDestroyDevice DestoryDevice;
    // ... 其他 Device 函数指针
} VkLayerDeviceDispatchTable;

extern std::unordered_map<VkInstance, VkLayerInstanceDispatchTable*> instance_dispatch;
extern std::unordered_map<VkDevice, VkLayerDeviceDispatchTable*> device_dispatch;
extern VkInstance* gInstance;
extern VkDevice* gDevice;

VkLayerDeviceCreateInfo *get_chain_info(const VkDeviceCreateInfo *pCreateInfo, VkLayerFunction func);
VkLayerInstanceCreateInfo *get_chain_info(const VkInstanceCreateInfo *pCreateInfo, VkLayerFunction func);

void init_device_dispatch_table(VkDevice device, VkLayerDeviceDispatchTable* table, PFN_vkGetDeviceProcAddr addr);
void init_instance_dispatch_table(VkInstance instance, VkLayerInstanceDispatchTable* table, PFN_vkGetInstanceProcAddr addr);