#include "Runtime/RHI/Public/Vulkan/VulkanDevice.h"
#include "Runtime/RHI/Public/Vulkan/VulkanBuffer.h"
#include "Runtime/RHI/Public/Vulkan/VulkanTexture.h"
#include "Runtime/RHI/Public/Vulkan/VulkanSwapchain.h"
#include "Runtime/RHI/Public/Vulkan/VulkanCommandList.h"
#include "Runtime/RHI/Public/Vulkan/Pipeline/VulkanShaderModule.h"
#include "Runtime/RHI/Public/Vulkan/Pipeline/VulkanGraphicsPipeline.h"
#include "Core/Logging/Public/Log.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan_win32.h>

namespace Sora::RHI
{
	VulkanDevice::VulkanDevice() = default;

	VulkanDevice::~VulkanDevice() {
		Shutdown();
	}

	bool VulkanDevice::Initialize(void* InNativeWindowHandle) {
		HWND WindowHandle = static_cast<HWND>(InNativeWindowHandle);
		if (!WindowHandle) {
			SORA_ENGINE_ERROR("VulkanDevice Initialize failed: Invalid HWND!");
			return false;
		}

		if (!CreateInstance()) return false;
		if (!CreateWin32Surface(WindowHandle)) return false;
		if (!SelectPhysicalDevice()) return false;
		if (!CreateLogicalDevice()) return false;
		if (!CreateVmaAllocator()) return false;
		if (!CreateSyncObjects()) return false;

		SORA_ENGINE_INFO("VulkanDevice initialized successfully for Win32 HWND: {0}", InNativeWindowHandle);
		return true;
	}

	bool VulkanDevice::CreateInstance() {
		VkApplicationInfo AppInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
		AppInfo.pApplicationName = "SoraEngine";
		AppInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
		AppInfo.pEngineName = "SoraEngine";
		AppInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
		AppInfo.apiVersion = VK_API_VERSION_1_3;

		const std::vector<const char*> Extensions = {
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME
		};

		VkInstanceCreateInfo CreateInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
		CreateInfo.pApplicationInfo = &AppInfo;
		CreateInfo.enabledExtensionCount = static_cast<uint32_t>(Extensions.size());
		CreateInfo.ppEnabledExtensionNames = Extensions.data();

		return vkCreateInstance(&CreateInfo, nullptr, &Instance) == VK_SUCCESS;
	}

	bool VulkanDevice::CreateWin32Surface(HWND InHwnd) {
		VkWin32SurfaceCreateInfoKHR SurfaceCreateInfo{ VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
		SurfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
		SurfaceCreateInfo.hwnd = InHwnd;

		if (vkCreateWin32SurfaceKHR(Instance, &SurfaceCreateInfo, nullptr, &Surface) != VK_SUCCESS) {
			SORA_ENGINE_ERROR("Failed to create Vulkan Win32 Surface!");
			return false;
		}
		return true;
	}

	bool VulkanDevice::SelectPhysicalDevice() {
		uint32_t DeviceCount = 0;
		vkEnumeratePhysicalDevices(Instance, &DeviceCount, nullptr);
		if (DeviceCount == 0) return false;

		std::vector<VkPhysicalDevice> Devices(DeviceCount);
		vkEnumeratePhysicalDevices(Instance, &DeviceCount, Devices.data());

		// Select discrete GPU if available, else pick first
		for (const auto& Dev : Devices) {
			VkPhysicalDeviceProperties Props;
			vkGetPhysicalDeviceProperties(Dev, &Props);
			if (Props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
				PhysicalDevice = Dev;
				break;
			}
		}

		if (PhysicalDevice == VK_NULL_HANDLE) {
			PhysicalDevice = Devices[0];
		}

		// Record device capabilities now that we've settled on a physical device
		VkPhysicalDeviceProperties SelectedProps;
		vkGetPhysicalDeviceProperties(PhysicalDevice, &SelectedProps);
		Capabilities.DeviceName = SelectedProps.deviceName;
		Capabilities.ApiVersion = SelectedProps.apiVersion;

		VkPhysicalDeviceMemoryProperties MemProps;
		vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &MemProps);
		for (uint32_t i = 0; i < MemProps.memoryHeapCount; ++i) {
			if (MemProps.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
				Capabilities.DedicatedVideoMemory = MemProps.memoryHeaps[i].size;
				break;
			}
		}

		// Find Queue Families
		uint32_t QueueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> QueueFamilies(QueueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyCount, QueueFamilies.data());

		for (uint32_t i = 0; i < QueueFamilyCount; ++i) {
			if (QueueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				GraphicsQueueFamilyIndex = i;
			}

			VkBool32 PresentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, i, Surface, &PresentSupport);
			if (PresentSupport) {
				PresentQueueFamilyIndex = i;
			}

			if (GraphicsQueueFamilyIndex != UINT32_MAX && PresentQueueFamilyIndex != UINT32_MAX) {
				break;
			}
		}

		return GraphicsQueueFamilyIndex != UINT32_MAX && PresentQueueFamilyIndex != UINT32_MAX;
	}

	bool VulkanDevice::CreateLogicalDevice() {
		std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos;
		std::vector<uint32_t> UniqueQueueFamilies = { GraphicsQueueFamilyIndex };
		if (GraphicsQueueFamilyIndex != PresentQueueFamilyIndex) {
			UniqueQueueFamilies.push_back(PresentQueueFamilyIndex);
		}

		float QueuePriority = 1.0f;
		for (uint32_t QueueFamily : UniqueQueueFamilies) {
			VkDeviceQueueCreateInfo QueueCreateInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
			QueueCreateInfo.queueFamilyIndex = QueueFamily;
			QueueCreateInfo.queueCount = 1;
			QueueCreateInfo.pQueuePriorities = &QueuePriority;
			QueueCreateInfos.push_back(QueueCreateInfo);
		}

		const std::vector<const char*> DeviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		// Enable Dynamic Rendering & Sync2 for Vulkan 1.3
		VkPhysicalDeviceVulkan13Features Features13{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
		Features13.dynamicRendering = VK_TRUE;
		Features13.synchronization2 = VK_TRUE;

		VkPhysicalDeviceFeatures2 DeviceFeatures2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
		DeviceFeatures2.pNext = &Features13;

		VkDeviceCreateInfo CreateInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
		CreateInfo.pNext = &DeviceFeatures2;
		CreateInfo.queueCreateInfoCount = static_cast<uint32_t>(QueueCreateInfos.size());
		CreateInfo.pQueueCreateInfos = QueueCreateInfos.data();
		CreateInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size());
		CreateInfo.ppEnabledExtensionNames = DeviceExtensions.data();

		if (vkCreateDevice(PhysicalDevice, &CreateInfo, nullptr, &LogicalDevice) != VK_SUCCESS) {
			SORA_ENGINE_ERROR("Failed to create Vulkan Logical Device!");
			return false;
		}

		vkGetDeviceQueue(LogicalDevice, GraphicsQueueFamilyIndex, 0, &GraphicsQueue);
		vkGetDeviceQueue(LogicalDevice, PresentQueueFamilyIndex, 0, &PresentQueue);

		// Command Pool setup
		VkCommandPoolCreateInfo PoolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		PoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		PoolInfo.queueFamilyIndex = GraphicsQueueFamilyIndex;
		vkCreateCommandPool(LogicalDevice, &PoolInfo, nullptr, &CommandPool);

		return true;
	}

	bool VulkanDevice::CreateVmaAllocator() {
		VmaAllocatorCreateInfo AllocatorCreateInfo{};
		AllocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
		AllocatorCreateInfo.physicalDevice = PhysicalDevice;
		AllocatorCreateInfo.device = LogicalDevice;
		AllocatorCreateInfo.instance = Instance;

		return vmaCreateAllocator(&AllocatorCreateInfo, &Allocator) == VK_SUCCESS;
	}

	bool VulkanDevice::CreateSyncObjects() {
		FrameInFlightFences.resize(MaxFramesInFlight);
		ImageAvailableSemaphores.resize(MaxFramesInFlight);
		RenderFinishedSemaphores.resize(MaxFramesInFlight);

		VkSemaphoreCreateInfo SemaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		VkFenceCreateInfo FenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		FenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MaxFramesInFlight; ++i) {
			if (vkCreateSemaphore(LogicalDevice, &SemaphoreInfo, nullptr, &ImageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(LogicalDevice, &SemaphoreInfo, nullptr, &RenderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(LogicalDevice, &FenceInfo, nullptr, &FrameInFlightFences[i]) != VK_SUCCESS) {
				return false;
			}
		}
		return true;
	}

	void VulkanDevice::BeginFrame() {
		vkWaitForFences(LogicalDevice, 1, &FrameInFlightFences[CurrentFrame], VK_TRUE, UINT64_MAX);
	}

	void VulkanDevice::EndFrame() {
		CurrentFrame = (CurrentFrame + 1) % MaxFramesInFlight;
	}

	void VulkanDevice::WaitIdle() {
		if (LogicalDevice != VK_NULL_HANDLE) {
			vkDeviceWaitIdle(LogicalDevice);
		}
	}

	std::shared_ptr<RHIBuffer> VulkanDevice::CreateBuffer(const RHIBufferDesc& InDesc) {
		return std::make_shared<VulkanBuffer>(Allocator, LogicalDevice, InDesc, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	}

	std::shared_ptr<RHITexture> VulkanDevice::CreateTexture(const RHITextureDesc& InDesc) {
		return std::make_shared<VulkanTexture>(Allocator, LogicalDevice, InDesc);
	}

	std::shared_ptr<RHISwapchain> VulkanDevice::CreateSwapchain(void* InWindowHandle, uint32_t InWidth, uint32_t InHeight) {
		RHISwapchainDesc Desc{};
		Desc.Width = InWidth;
		Desc.Height = InHeight;

		Swapchain = std::make_shared<VulkanSwapchain>(this, InWidth, InHeight, Desc);

		return Swapchain;
	}

	std::shared_ptr<RHICommandList> VulkanDevice::CreateCommandList() {
		return std::make_shared<VulkanCommandList>(this, CommandPool);
	}

	std::shared_ptr<RHIShaderModule> VulkanDevice::CreateShaderModule(const RHIShaderModuleDesc& InDesc) {
		return std::make_shared<VulkanShaderModule>(LogicalDevice, InDesc);
	}

	std::shared_ptr<RHIGraphicsPipeline> VulkanDevice::CreateGraphicsPipeline(const RHIGraphicsPipelineDesc& InDesc) {
		return std::make_shared<VulkanGraphicsPipeline>(LogicalDevice, InDesc);
	}

	void VulkanDevice::Shutdown() {
		WaitIdle();

		for (size_t i = 0; i < MaxFramesInFlight; ++i) {
			if (i < ImageAvailableSemaphores.size() && ImageAvailableSemaphores[i]) vkDestroySemaphore(LogicalDevice, ImageAvailableSemaphores[i], nullptr);
			if (i < RenderFinishedSemaphores.size() && RenderFinishedSemaphores[i]) vkDestroySemaphore(LogicalDevice, RenderFinishedSemaphores[i], nullptr);
			if (i < FrameInFlightFences.size() && FrameInFlightFences[i]) vkDestroyFence(LogicalDevice, FrameInFlightFences[i], nullptr);
		}

		if (CommandPool) vkDestroyCommandPool(LogicalDevice, CommandPool, nullptr);
		if (Allocator) vmaDestroyAllocator(Allocator);
		if (LogicalDevice) vkDestroyDevice(LogicalDevice, nullptr);
		if (Surface) vkDestroySurfaceKHR(Instance, Surface, nullptr);
		if (Instance) vkDestroyInstance(Instance, nullptr);
	}
} // namespace Sora::RHI
