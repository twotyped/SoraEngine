#pragma once

#include "Runtime/RHI/Public/RHISwapchain.h"
#include "VulkanDevice.h"
#include <vulkan/vulkan.h>
#include <vector>

namespace Sora::RHI
{

	class VulkanSwapchain : public RHISwapchain {
	public:
		VulkanSwapchain(
			VulkanDevice* InDevice,
			uint32_t InWidth,
			uint32_t InHeight,
			const RHISwapchainDesc& InDesc,
			ERHIPresentMode InPresentMode = ERHIPresentMode::VSync
		) : Device(InDevice), Desc(InDesc), PresentMode(InPresentMode)
		{
			LogicalDevice = Device->GetVkDevice();
			PhysicalDevice = Device->GetVkPhysicalDevice();
			Surface = Device->GetVkSurface();
			CreateSwapchain();
		}

		virtual ~VulkanSwapchain() override {
			CleanupSwapchain();
		};

		virtual bool AcquireNextImage() override;
		bool Present() override;
		void Resize(uint32_t InWidth, uint32_t InHeight) override;

		std::shared_ptr<RHITexture> GetCurrentBackBuffer() const override;

		uint32_t GetCurrentImageIndex() const override { return CurrentImageIndex; }
		const RHISwapchainDesc& GetDesc() const override { return Desc; }

	private:
		bool CreateSwapchain();
		void DestroyImageViews();
		void CleanupSwapchain();

		VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& AvailableFormats);
		VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& AvailablePresentModes);
		VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& Capabilities, uint32_t InWidth, uint32_t InHeight);

		RHISwapchainDesc Desc;

		VulkanDevice* Device{ nullptr };

		VkDevice LogicalDevice{ VK_NULL_HANDLE };
		VkPhysicalDevice PhysicalDevice{ VK_NULL_HANDLE };
		VkSurfaceKHR Surface{ VK_NULL_HANDLE };

		VkSwapchainKHR Swapchain{ VK_NULL_HANDLE };
		VkSwapchainKHR OldSwapchain{ VK_NULL_HANDLE };
		std::vector<VkImage> SwapchainImages;
		std::vector<VkImageView> SwapchainImageViews;
		std::vector<std::shared_ptr<RHITexture>> BackBuffers;
		uint32_t CurrentImageIndex{ 0 };
		uint32_t CurrentFrameIndex{ 0 };
		VkFormat SwapchainImageFormat{ VK_FORMAT_UNDEFINED };
		VkColorSpaceKHR SwapchainColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		VkExtent2D SwapchainExtent{ 0, 0 };

		ERHIPresentMode PresentMode{ ERHIPresentMode::VSync };

		std::vector<VkSemaphore> ImageAvailableSemaphores;
		std::vector<VkSemaphore> RenderFinishedSemaphores;
	};
} // namespace Sora::RHI
