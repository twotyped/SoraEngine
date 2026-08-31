#include "Runtime/RHI/Public/Devices/Vulkan/VulkanSwapchain.h"
#include "Runtime/RHI/Public/Devices/Vulkan/VulkanTexture.h"
#include "Runtime/RHI/Public/RHITypes.h"
#include "Core/Logging/Public/Log.h"

namespace Sora::RHI
{

	VulkanSwapchain::VulkanSwapchain(VkDevice InDevice, VkPhysicalDevice InPhysicalDevice, VkSurfaceKHR InSurface, uint32_t InWidth, uint32_t InHeight)
		: LogicalDevice(InDevice), PhysicalDevice(InPhysicalDevice), Surface(InSurface) {
		CreateSwapchainInternal(InWidth, InHeight);

		VkSemaphoreCreateInfo SemaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		VkFenceCreateInfo FenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		FenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		ImageAvailableSemaphores.resize(MaxFramesInFlight);
		RenderFinishedSemaphores.resize(MaxFramesInFlight);
		InFlightFences.resize(MaxFramesInFlight);

		for (uint32_t Index = 0; Index < MaxFramesInFlight; ++Index) {
			vkCreateSemaphore(LogicalDevice, &SemaphoreInfo, nullptr, &ImageAvailableSemaphores[Index]);
			vkCreateSemaphore(LogicalDevice, &SemaphoreInfo, nullptr, &RenderFinishedSemaphores[Index]);
			vkCreateFence(LogicalDevice, &FenceInfo, nullptr, &InFlightFences[Index]);
		}
	}

	VulkanSwapchain::~VulkanSwapchain() {
		Cleanup();

		for (uint32_t Index = 0; Index < MaxFramesInFlight; ++Index) {
			vkDestroySemaphore(LogicalDevice, ImageAvailableSemaphores[Index], nullptr);
			vkDestroySemaphore(LogicalDevice, RenderFinishedSemaphores[Index], nullptr);
			vkDestroyFence(LogicalDevice, InFlightFences[Index], nullptr);
		}
	}

	void VulkanSwapchain::CreateSwapchainInternal(uint32_t InWidth, uint32_t InHeight) {
		vkb::SwapchainBuilder Builder{ PhysicalDevice, LogicalDevice, Surface };
		auto SwapchainResult = Builder
			.set_desired_extent(InWidth, InHeight)
			.set_desired_format({ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
			.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR) // VSync enabled
			.set_old_swapchain(Swapchain)
			.build();

		if (!SwapchainResult) {
			SORA_ENGINE_ERROR("Failed to create Vulkan Swapchain: {0}", SwapchainResult.error().message());
			return;
		}

		SwapchainTextures.clear();
		for (size_t Index = 0; Index < SwapchainImages.size(); ++Index) {
			RHITextureDesc BackBufferDesc{};
			BackBufferDesc.Width = InWidth;
			BackBufferDesc.Height = InHeight;
			BackBufferDesc.Format = RHIFormat::B8G8R8A8_UNORM;
			BackBufferDesc.Usage = RHITextureUsage::ColorAttachment;

			SwapchainTextures.push_back(std::make_shared<VulkanTexture>(
				LogicalDevice,
				SwapchainImages[Index],
				SwapchainImageViews[Index],
				BackBufferDesc
			));
		}

		VkbSwapchain = SwapchainResult.value();
		Swapchain = VkbSwapchain.swapchain;
		SwapchainImages = VkbSwapchain.get_images().value();
		SwapchainImageViews = VkbSwapchain.get_image_views().value();

		Desc.Width = InWidth;
		Desc.Height = InHeight;
		Desc.BufferCount = static_cast<uint32_t>(SwapchainImages.size());
	}

	bool VulkanSwapchain::AcquireNextImage() {
		vkWaitForFences(LogicalDevice, 1, &InFlightFences[CurrentFrameIndex], VK_TRUE, UINT64_MAX);

		VkResult AcquireResult = vkAcquireNextImageKHR(
			LogicalDevice,
			Swapchain,
			UINT64_MAX,
			ImageAvailableSemaphores[CurrentFrameIndex],
			VK_NULL_HANDLE,
			&CurrentImageIndex
		);

		if (AcquireResult == VK_ERROR_OUT_OF_DATE_KHR) {
			return false; // Triggers swapchain resize logic
		}

		vkResetFences(LogicalDevice, 1, &InFlightFences[CurrentFrameIndex]);
		return AcquireResult == VK_SUCCESS || AcquireResult == VK_SUBOPTIMAL_KHR;
	}

	bool VulkanSwapchain::Present() {
		VkPresentInfoKHR PresentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		PresentInfo.waitSemaphoreCount = 1;
		PresentInfo.pWaitSemaphores = &RenderFinishedSemaphores[CurrentFrameIndex];
		PresentInfo.swapchainCount = 1;
		PresentInfo.pSwapchains = &Swapchain;
		PresentInfo.pImageIndices = &CurrentImageIndex;

		// Queue submission takes place before present; placeholder assuming graphics/present queue
		VkQueue PresentQueue = VK_NULL_HANDLE; 
		vkGetDeviceQueue(LogicalDevice, 0, 0, &PresentQueue); // Adjust queue handle retrieval as needed

		VkResult PresentResult = vkQueuePresentKHR(PresentQueue, &PresentInfo);

		CurrentFrameIndex = (CurrentFrameIndex + 1) % MaxFramesInFlight;

		if (PresentResult == VK_ERROR_OUT_OF_DATE_KHR || PresentResult == VK_SUBOPTIMAL_KHR) {
			return false;
		}

		return PresentResult == VK_SUCCESS;
	}

	void VulkanSwapchain::Resize(uint32_t InWidth, uint32_t InHeight) {
		vkDeviceWaitIdle(LogicalDevice);
		Cleanup();
		CreateSwapchainInternal(InWidth, InHeight);
	}

	void VulkanSwapchain::Cleanup() {
		for (auto ImageView : SwapchainImageViews) {
			vkDestroyImageView(LogicalDevice, ImageView, nullptr);
		}
		SwapchainImageViews.clear();

		if (Swapchain != VK_NULL_HANDLE) {
			vkb::destroy_swapchain(VkbSwapchain);
			Swapchain = VK_NULL_HANDLE;
		}
	}

	std::shared_ptr<RHITexture> VulkanSwapchain::GetCurrentBackBuffer() const {
		if (CurrentImageIndex < SwapchainTextures.size()) {
			return SwapchainTextures[CurrentImageIndex];
		}
		return nullptr;
	}

} // namespace Sora::RHI