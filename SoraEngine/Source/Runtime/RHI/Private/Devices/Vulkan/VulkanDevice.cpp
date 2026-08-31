#include "Runtime/RHI/Public/Devices/Vulkan/VulkanBuffer.h"
#include "Runtime/RHI/Public/Devices/Vulkan/VulkanSwapchain.h"
#include "Runtime/RHI/Public/Devices/Vulkan/VulkanDevice.h"
#include "Core/Logging/Public/Log.h"

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace Sora::RHI
{

	bool VulkanDevice::Initialize(void* InNativeWindowHandle) {
		// 1. Create Vulkan Instance
		vkb::InstanceBuilder InstanceBuilder;
		auto InstanceResult = InstanceBuilder
			.set_app_name("SoraEngine")
			.request_validation_layers(true)
			.use_default_debug_messenger()
			.require_api_version(1, 3, 0)
			.build();

		if (!InstanceResult) {
			SORA_ENGINE_ERROR("Failed to create Vulkan Instance: {0}", InstanceResult.error().message());
            return false;
		}

		VkbInstance = InstanceResult.value();
		Instance = VkbInstance.instance;
		DebugMessenger = VkbInstance.debug_messenger;

		SORA_ENGINE_INFO("Vulkan 1.3 instance created successfully.");

#if SORA_PLATFORM_WINDOWS
		VkWin32SurfaceCreateInfoKHR SurfaceCreateInfo{};
		SurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		SurfaceCreateInfo.hwnd = static_cast<HWND>(InNativeWindowHandle);
		SurfaceCreateInfo.hinstance = GetModuleHandle(nullptr);

		if (vkCreateWin32SurfaceKHR(Instance, &SurfaceCreateInfo, nullptr, &Surface) != VK_SUCCESS) {
			SORA_ENGINE_ERROR("Failed to create Win32 Vulkan Surface!");
			return false;
		}
#else
#error "SoraEngine only supports Windows as of now."
#endif

		// 2. Select physical device with the required features
        VkPhysicalDeviceVulkan13Features Features13{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
        Features13.dynamicRendering = VK_TRUE;
        Features13.synchronization2 = VK_TRUE;

		vkb::PhysicalDeviceSelector Selector{ VkbInstance };
		auto PhysDeviceResult = Selector
			.set_minimum_version(1, 3)
			.set_surface(Surface)
            .set_required_features_13(Features13)
            .select();

		if (!PhysDeviceResult) {
            SORA_ENGINE_ERROR("Failed to find suitable Vulkan Physical Device: {0}", PhysDeviceResult.error().message());
            return false;
        }

		vkb::PhysicalDevice vkbPhysicalDevice = PhysDeviceResult.value();
		PhysicalDevice = vkbPhysicalDevice.physical_device;

		// 3. Populate device capabilities
		Capabilities.DeviceName = vkbPhysicalDevice.properties.deviceName;
        Capabilities.ApiVersion = vkbPhysicalDevice.properties.apiVersion;
        Capabilities.SupportsBindless = true;
        
        SORA_ENGINE_INFO("Selected GPU: {0}", Capabilities.DeviceName);

		// 4. Create logical device
		vkb::DeviceBuilder DeviceBuilder{ vkbPhysicalDevice };
		auto DeviceResult = DeviceBuilder.build();

		if (!DeviceResult) {
			SORA_ENGINE_ERROR("Failed to create a Vulkan Logical Device: {0}", DeviceResult.error().message());
			return false;
		}

		vkb::Device vkbDevice = DeviceResult.value();
		LogicalDevice = vkbDevice.device;

		// 5. Get queues and queue families
		auto GraphicsQueueResult = vkbDevice.get_queue(vkb::QueueType::graphics);
		if (!GraphicsQueueResult) {
			SORA_ENGINE_ERROR("Failed to get Graphics Queue: {0}", GraphicsQueueResult.error().message());
		}

		GraphicsQueue = GraphicsQueueResult.value();
		GraphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

		auto PresentQueueResult = vkbDevice.get_queue(vkb::QueueType::present);
		if (PresentQueueResult) {
			PresentQueue = PresentQueueResult.value();
			PresentQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::present).value();
		} else {
			SORA_ENGINE_TRACE("Defaulting back to Graphics queue.");
			PresentQueue = GraphicsQueue;
			PresentQueueFamily = GraphicsQueueFamily;
		}

		// 6. Initialize the Vulkan Memory Allocator (VMA)
		VmaAllocatorCreateInfo AllocatorCreateInfo{};
        AllocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
        AllocatorCreateInfo.physicalDevice = PhysicalDevice;
        AllocatorCreateInfo.device = LogicalDevice;
        AllocatorCreateInfo.instance = Instance;

		if (vmaCreateAllocator(&AllocatorCreateInfo, &Allocator) != VK_SUCCESS) {
			SORA_ENGINE_ERROR("Failed to create Vulkan Memory Allocator (VMA)!");
			return false;
		}

		// 7. Initialize the command list
		VkCommandPoolCreateInfo PoolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		PoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		PoolInfo.queueFamilyIndex = GraphicsQueueFamily;

		if (vkCreateCommandPool(LogicalDevice, &PoolInfo, nullptr, &CommandPool) != VK_SUCCESS) {
			SORA_ENGINE_ERROR("Failed to create Vulkan Command Pool!");
			return false;
		}

		MainCommandList = std::make_shared<VulkanCommandList>(LogicalDevice, CommandPool);

		SORA_ENGINE_INFO("Vulkan Device & VMA initialized successfully.");
		return true;
	}

	void VulkanDevice::Shutdown() {
		WaitIdle();

		if (Allocator != VK_NULL_HANDLE) {
			vmaDestroyAllocator(Allocator);
			Allocator = VK_NULL_HANDLE;
		}

		if (CommandPool != VK_NULL_HANDLE) {
			vkDestroyCommandPool(LogicalDevice, CommandPool, nullptr);
			CommandPool = VK_NULL_HANDLE;
		}

		if (LogicalDevice != VK_NULL_HANDLE)
        {
            vkDestroyDevice(LogicalDevice, nullptr);
            LogicalDevice = VK_NULL_HANDLE;
        }

        if (Instance != VK_NULL_HANDLE)
        {
            vkb::destroy_instance(VkbInstance);
            Instance = VK_NULL_HANDLE;
        }

		SORA_ENGINE_INFO("Vulkan Device shutdown complete.");
	}

	void VulkanDevice::WaitIdle() {
		if (LogicalDevice != VK_NULL_HANDLE) {
			vkDeviceWaitIdle(LogicalDevice);
		}
	}

	void VulkanDevice::BeginFrame() {
		if (MainCommandList) {
			MainCommandList->Begin();
		}
	}

	void VulkanDevice::EndFrame() {
		if (!MainCommandList) {
			return;
		}

		MainCommandList->End();

		VkCommandBuffer CmdBuf = MainCommandList->GetVkCommandBuffer();

		VkSubmitInfo SubmitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		SubmitInfo.commandBufferCount = 1;
		SubmitInfo.pCommandBuffers = &CmdBuf;

		if (vkQueueSubmit(GraphicsQueue, 1, &SubmitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
			SORA_ENGINE_ERROR("Failed to submit Vulkan Command Buffer to Graphics Queue!");
		}
	}

	std::shared_ptr<RHIBuffer> VulkanDevice::CreateBuffer(const RHIBufferDesc& InDesc) {
		return std::make_shared<VulkanBuffer>(Allocator, InDesc);
	}

	std::shared_ptr<RHISwapchain> VulkanDevice::CreateSwapchain(void* InNativeWindowHandle, uint32_t InWidth, uint32_t InHeight) {
		// Uses the HWND and VkSurfaceKHR initialized on device startup
		return std::make_shared<VulkanSwapchain>(LogicalDevice, PhysicalDevice, Surface, InWidth, InHeight);
	}

	std::shared_ptr<RHICommandList> VulkanDevice::CreateCommandList() {
		return std::make_shared<VulkanCommandList>(LogicalDevice, CommandPool);
	}
} // namespace Sora::RHI