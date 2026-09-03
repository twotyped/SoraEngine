#pragma once

#if defined(_WIN32)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
    #define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

#include "Runtime/RHI/Public/RHIDevice.h"

#include <vector>
#include <memory>

namespace Sora::RHI
{
	class VulkanSwapchain;

	class VulkanDevice : public RHIDevice {
	public:
		VulkanDevice();
		virtual ~VulkanDevice() override;

		virtual bool Initialize(void* InNativeWindowHandle) override;
		virtual void Shutdown() override;
		
		virtual const RHIDeviceCapabilities& GetCapabilities() const override { return Capabilities; }

		// Resource Factories
		virtual std::shared_ptr<RHIBuffer> CreateBuffer(const RHIBufferDesc& InDesc) override;
		virtual std::shared_ptr<RHITexture> CreateTexture(const RHITextureDesc& InDesc) override;
		virtual std::shared_ptr<RHISwapchain> CreateSwapchain(void* InWindowHandle, uint32_t InWidth, uint32_t InHeight) override;
		virtual std::shared_ptr<RHICommandList> CreateCommandList() override;
		virtual std::shared_ptr<RHIShaderModule> CreateShaderModule(const RHIShaderModuleDesc& InDesc) override;
		virtual std::shared_ptr<RHIGraphicsPipeline> CreateGraphicsPipeline(const RHIGraphicsPipelineDesc& InDesc) override;

		// Frame Lifecycle
		virtual void BeginFrame() override;
		virtual void EndFrame() override;
		virtual void WaitIdle() override;

		// Frame Sync Accessors
		VkSemaphore GetCurrentImageAvailableSemaphore() const { return ImageAvailableSemaphores[CurrentFrame]; }
		VkSemaphore GetCurrentRenderFinishedSemaphore() const { return RenderFinishedSemaphores[CurrentFrame]; }
		VkFence GetCurrentFrameFence() const { return FrameInFlightFences[CurrentFrame]; }
		uint32_t GetCurrentFrameIndex() const { return CurrentFrame; }

		// Vulkan Getters
		VkDevice GetVkDevice() const { return LogicalDevice; }
		VkPhysicalDevice GetVkPhysicalDevice() const { return PhysicalDevice; }
		VkInstance GetVkInstance() const { return Instance; }
		VkSurfaceKHR GetVkSurface() const { return Surface; }
		VkQueue GetGraphicsQueue() const { return GraphicsQueue; }
		VkQueue GetPresentQueue() const { return PresentQueue; }
		uint32_t GetGraphicsQueueFamilyIndex() const { return GraphicsQueueFamilyIndex; }
		uint32_t GetPresentQueueFamilyIndex() const { return PresentQueueFamilyIndex; }
		VmaAllocator GetVmaAllocator() const { return Allocator; }

	private:
		bool CreateInstance();
		bool CreateWin32Surface(HWND InHwnd);
		bool SelectPhysicalDevice();
		bool CreateLogicalDevice();
		bool CreateVmaAllocator();
		bool CreateSyncObjects();

	private:
		VkInstance Instance{ VK_NULL_HANDLE };
		VkPhysicalDevice PhysicalDevice{ VK_NULL_HANDLE };
		VkDevice LogicalDevice{ VK_NULL_HANDLE };
		VkSurfaceKHR Surface{ VK_NULL_HANDLE };

		VkQueue GraphicsQueue{ VK_NULL_HANDLE };
		VkQueue PresentQueue{ VK_NULL_HANDLE };
		uint32_t GraphicsQueueFamilyIndex{ UINT32_MAX };
		uint32_t PresentQueueFamilyIndex{ UINT32_MAX };
		
		VmaAllocator Allocator{ VK_NULL_HANDLE };
		RHIDeviceCapabilities Capabilities{};

		static constexpr uint32_t MaxFramesInFlight = 2;
		uint32_t CurrentFrame = 0;

		std::vector<VkFence> FrameInFlightFences;
		std::vector<VkSemaphore> ImageAvailableSemaphores;
		std::vector<VkSemaphore> RenderFinishedSemaphores;
		VkCommandPool CommandPool{ VK_NULL_HANDLE };
	};

} // namespace Sora::RHI
