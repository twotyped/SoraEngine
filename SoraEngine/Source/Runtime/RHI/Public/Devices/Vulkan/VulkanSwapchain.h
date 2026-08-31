#pragma once

#include "Runtime/RHI/Public/RHISwapchain.h"
#include <vulkan/vulkan.h>
#include <VkBootstrap.h>
#include <vector>
#include <memory>

namespace Sora::RHI
{

	class VulkanSwapchain : public RHISwapchain {
	public:
		VulkanSwapchain(VkDevice InDevice, VkPhysicalDevice InPhysicalDevice, VkSurfaceKHR InSurface, uint32_t InWidth, uint32_t InHeight);
		virtual ~VulkanSwapchain() override;

		virtual bool AcquireNextImage() override;
		virtual bool Present() override;
		virtual void Resize(uint32_t InWidth, uint32_t InHeight) override;

		virtual std::shared_ptr<RHITexture> GetCurrentBackBuffer() const override;
		virtual uint32_t GetCurrentImageIndex() const override { return CurrentImageIndex; }
		virtual const RHISwapchainDesc& GetDesc() const override { return Desc; }

		VkSwapchainKHR GetVkSwapchain() const { return Swapchain; }
		VkSemaphore GetCurrentImageAvailableSemaphore() const { return ImageAvailableSemaphores[CurrentFrameIndex]; }
		VkSemaphore GetCurrentRenderFinishedSemaphore() const { return RenderFinishedSemaphores[CurrentFrameIndex]; }

	private:
		void CreateSwapchainInternal(uint32_t InWidth, uint32_t InHeight);
		void Cleanup();

		VkDevice LogicalDevice{ VK_NULL_HANDLE };
		VkPhysicalDevice PhysicalDevice{ VK_NULL_HANDLE };
		VkSurfaceKHR Surface{ VK_NULL_HANDLE };

		vkb::Swapchain VkbSwapchain{};
		VkSwapchainKHR Swapchain{ VK_NULL_HANDLE };

		std::vector<VkImage> SwapchainImages;
		std::vector<VkImageView> SwapchainImageViews;

		std::vector<std::shared_ptr<RHITexture>> SwapchainTextures;

		std::vector<VkSemaphore> ImageAvailableSemaphores;
		std::vector<VkSemaphore> RenderFinishedSemaphores;
		std::vector<VkFence> InFlightFences;

		uint32_t CurrentImageIndex{ 0 };
		uint32_t CurrentFrameIndex{ 0 };
		static constexpr uint32_t MaxFramesInFlight{ 2 };
		RHISwapchainDesc Desc{};
	};

} // namespace Sora::RHI
