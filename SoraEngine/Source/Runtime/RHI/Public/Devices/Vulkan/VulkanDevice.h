#pragma once

#include "Runtime/RHI/Public/RHIDevice.h"
#include "Runtime/RHI/Public/Devices/Vulkan/VulkanCommandList.h"

#ifdef SORA_PLATFORM_WINDOWS
#define VK_USE_PLATFORM_WIN32_KHR
#else
#error "SoraEngine only supports windows as of now."
#endif
#include <vulkan/vulkan.h>
#include <VkBootstrap.h>
#include <vma/vk_mem_alloc.h>

namespace Sora::RHI
{

	class SORA_API VulkanDevice : public RHIDevice {
	public:
		VulkanDevice() = default;
		virtual ~VulkanDevice() override = default;

		virtual bool Initialize(void* InNativeWindowHandle) override;
        virtual void Shutdown() override;

        virtual const RHIDeviceCapabilities& GetCapabilities() const override { return Capabilities; }

        virtual std::shared_ptr<RHIBuffer> CreateBuffer(const RHIBufferDesc& InDesc) override;
        virtual std::shared_ptr<RHISwapchain> CreateSwapchain(void* InNativeWindowHandle, uint32_t InWidth, uint32_t InHeight) override;
        virtual std::shared_ptr<RHICommandList> CreateCommandList() override;

        virtual void BeginFrame() override;
        virtual void EndFrame() override;
        virtual void WaitIdle() override;

        VkInstance GetVkInstance() const { return Instance; }
        VkDevice GetVkDevice() const { return LogicalDevice; }
        VkPhysicalDevice GetVkPhysicalDevice() const { return PhysicalDevice; }
        VmaAllocator GetVmaAllocator() const { return Allocator; }

	private:
        vkb::Instance VkbInstance{};
        VkInstance Instance{ VK_NULL_HANDLE };
        VkDebugUtilsMessengerEXT DebugMessenger{ VK_NULL_HANDLE };

        VkSurfaceKHR Surface;

        VkPhysicalDevice PhysicalDevice{ VK_NULL_HANDLE };
        VkDevice LogicalDevice{ VK_NULL_HANDLE };
        VmaAllocator Allocator{ VK_NULL_HANDLE };

        VkQueue GraphicsQueue{ VK_NULL_HANDLE };
        uint32_t GraphicsQueueFamily{ 0 };

        VkQueue PresentQueue{ VK_NULL_HANDLE };
        uint32_t PresentQueueFamily{ 0 };

        RHIDeviceCapabilities Capabilities{};

        VkCommandPool CommandPool{ VK_NULL_HANDLE };
		std::shared_ptr<VulkanCommandList> MainCommandList{ nullptr };
	};

};
