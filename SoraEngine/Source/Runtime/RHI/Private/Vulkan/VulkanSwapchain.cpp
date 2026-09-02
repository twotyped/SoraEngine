#include "Runtime/RHI/Public/Vulkan/VulkanSwapchain.h"
#include "Runtime/RHI/Public/Vulkan/VulkanTexture.h"
#include "Runtime/RHI/Public/RHITexture.h"
#include "Core/Logging/Public/Log.h"

namespace Sora::RHI
{

	bool VulkanSwapchain::AcquireNextImage() {
			VkResult result = vkAcquireNextImageKHR(
				LogicalDevice,
				Swapchain,
				UINT64_MAX,
				ImageAvailableSemaphores[CurrentFrameIndex],
				VK_NULL_HANDLE,
				&CurrentImageIndex
			);

			if (result == VK_ERROR_OUT_OF_DATE_KHR) {
				return false;
			}

			return (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);
	}

	bool VulkanSwapchain::Present() {
		VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &RenderFinishedSemaphores[CurrentFrameIndex];
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &Swapchain;
        presentInfo.pImageIndices = &CurrentImageIndex;

		VkResult result = vkQueuePresentKHR(Device->GetPresentQueue(), &presentInfo);

		uint32_t MaxFrames = ImageAvailableSemaphores.empty() ? 1 : static_cast<uint32_t>(ImageAvailableSemaphores.size());
		CurrentFrameIndex = (CurrentFrameIndex + 1) % MaxFrames;

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			SORA_ENGINE_ERROR("Swapchain is out of date. Need to recreate swapchain.");
			return false;
		} else if (result == VK_SUBOPTIMAL_KHR) {
			SORA_ENGINE_WARN("Swapchain is suboptimal. Consider recreating swapchain.");
			return true;
		} else if (result != VK_SUCCESS) {
			SORA_ENGINE_ERROR("Failed to present swapchain image!");
			return false;
		}

		return true;
	}

	void VulkanSwapchain::Resize(uint32_t InWidth, uint32_t InHeight) {
		if (InWidth == 0 || InHeight == 0) return;
		Desc.Width = InWidth;
		Desc.Height = InHeight;

		Device->WaitIdle();
		CreateSwapchain();
	}

	std::shared_ptr<RHITexture> VulkanSwapchain::GetCurrentBackBuffer() const {
		if (CurrentImageIndex < BackBuffers.size()) {
			return BackBuffers[CurrentImageIndex];
		}
		return nullptr;
	}

	void VulkanSwapchain::CleanupSwapchain() {
        DestroyImageViews();
        BackBuffers.clear();

        if (Swapchain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(LogicalDevice, Swapchain, nullptr);
            Swapchain = VK_NULL_HANDLE;
        }

        for (size_t i = 0; i < ImageAvailableSemaphores.size(); ++i) {
            if (ImageAvailableSemaphores[i] != VK_NULL_HANDLE) {
                vkDestroySemaphore(LogicalDevice, ImageAvailableSemaphores[i], nullptr);
            }
            if (i < RenderFinishedSemaphores.size() && RenderFinishedSemaphores[i] != VK_NULL_HANDLE) {
                vkDestroySemaphore(LogicalDevice, RenderFinishedSemaphores[i], nullptr);
            }
        }

        ImageAvailableSemaphores.clear();
        RenderFinishedSemaphores.clear();
    }

	void VulkanSwapchain::DestroyImageViews() {
        for (auto view : SwapchainImageViews) {
            if (view != VK_NULL_HANDLE) {
                vkDestroyImageView(LogicalDevice, view, nullptr);
            }
        }
        SwapchainImageViews.clear();
    }

	VkSurfaceFormatKHR VulkanSwapchain::ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& AvailableFormats) {
		if (AvailableFormats.empty()) {
			SORA_ENGINE_WARN("No available surface formats found! Selecting defaults.");
			return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		}

		if (Desc.EnableHDR) {
			// 1. Try 10-bit HDR10 (PQ Transfer Function)
			for (const auto& format : AvailableFormats) {
				if (format.format == VK_FORMAT_A2B10G10R10_UNORM_PACK32 && 
					format.colorSpace == VK_COLOR_SPACE_HDR10_ST2084_EXT) {
					return format;
				}
			}

			// 2. Fallback to 16-bit scRGB (Linear Transfer Function)
			for (const auto& format : AvailableFormats) {
				if (format.format == VK_FORMAT_R16G16B16A16_SFLOAT && 
					format.colorSpace == VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT) {
					return format;
				}
			}

			SORA_ENGINE_WARN("HDR requested but no supported HDR format found. Falling back to SDR.");
		}

		auto it = std::ranges::find_if(AvailableFormats, [](const VkSurfaceFormatKHR& format) {
        return format.format == VK_FORMAT_B8G8R8A8_UNORM && 
               format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		});

		return (it != AvailableFormats.end()) ? *it : AvailableFormats[0];
	}

	VkPresentModeKHR VulkanSwapchain::ChoosePresentMode(const std::vector<VkPresentModeKHR>& AvailablePresentModes) {
		VkPresentModeKHR DesiredMode = VK_PRESENT_MODE_FIFO_KHR; // Default to VSync

		if (PresentMode == ERHIPresentMode::Immediate) {
			DesiredMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		} else if (PresentMode == ERHIPresentMode::Mailbox) {
			DesiredMode = VK_PRESENT_MODE_MAILBOX_KHR;
		}

		bool bSupported = std::ranges::any_of(AvailablePresentModes, [DesiredMode](VkPresentModeKHR mode) {
			return mode == DesiredMode;
		});

		if (bSupported) {
			return DesiredMode;
		}

		return VK_PRESENT_MODE_FIFO_KHR; // Fallback to VSync if desired mode is not supported
	}

	VkExtent2D VulkanSwapchain::ChooseExtent(const VkSurfaceCapabilitiesKHR& Capabilities, uint32_t InWidth, uint32_t InHeight) {
		if (Capabilities.currentExtent.width != UINT32_MAX) {
			return Capabilities.currentExtent;
		}

		VkExtent2D actualExtent = { InWidth, InHeight };
		actualExtent.width = std::clamp(actualExtent.width, Capabilities.minImageExtent.width, Capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, Capabilities.minImageExtent.height, Capabilities.maxImageExtent.height);

		return actualExtent;
	}

	bool VulkanSwapchain::CreateSwapchain() { 
		VkSurfaceCapabilitiesKHR Capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, Surface, &Capabilities);

		uint32_t FormatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &FormatCount, nullptr);
		std::vector<VkSurfaceFormatKHR> Formats(FormatCount);
		if (FormatCount > 0) {
			vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &FormatCount, Formats.data());
		}

		uint32_t PresentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &PresentModeCount, nullptr);
		std::vector<VkPresentModeKHR> PresentModes(PresentModeCount);
		if (PresentModeCount > 0) {
			vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &PresentModeCount, PresentModes.data());
		}

		VkSurfaceFormatKHR SurfaceFormat = ChooseSurfaceFormat(Formats);
		VkPresentModeKHR PresentMode = ChoosePresentMode(PresentModes);
		VkExtent2D Extent = ChooseExtent(Capabilities, Desc.Width, Desc.Height);

		SwapchainImageFormat = SurfaceFormat.format;
		SwapchainColorSpace = SurfaceFormat.colorSpace;
		SwapchainExtent = Extent;

		uint32_t ImageCount = Capabilities.minImageCount + 1;
		if (Capabilities.maxImageCount > 0 && ImageCount > Capabilities.maxImageCount) {
			SORA_ENGINE_WARN("Requested image count {0} exceeds max image count {1}. Clamping to max.", ImageCount, Capabilities.maxImageCount);
			ImageCount = Capabilities.maxImageCount;
		}

		Desc.BufferCount = ImageCount;

		OldSwapchain = Swapchain;

		VkSwapchainCreateInfoKHR CreateInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
		CreateInfo.surface = Surface;
		CreateInfo.minImageCount = ImageCount;
		CreateInfo.imageFormat = SwapchainImageFormat;
		CreateInfo.imageColorSpace = SwapchainColorSpace;
		CreateInfo.imageExtent = SwapchainExtent;
		CreateInfo.imageArrayLayers = 1;
		CreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		CreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		CreateInfo.preTransform = Capabilities.currentTransform;
		CreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		CreateInfo.presentMode = PresentMode;
		CreateInfo.clipped = VK_TRUE;
		CreateInfo.oldSwapchain = OldSwapchain;

		if (vkCreateSwapchainKHR(LogicalDevice, &CreateInfo, nullptr, &Swapchain) != VK_SUCCESS) {
			SORA_ENGINE_ERROR("Failed to create swapchain!");
			return false;
		}

		DestroyImageViews();
        BackBuffers.clear();
		if (OldSwapchain != VK_NULL_HANDLE) {
			vkDestroySwapchainKHR(LogicalDevice, OldSwapchain, nullptr);
			OldSwapchain = VK_NULL_HANDLE;
		}

		vkGetSwapchainImagesKHR(LogicalDevice, Swapchain, &ImageCount, nullptr);
		SwapchainImages.resize(ImageCount);
		vkGetSwapchainImagesKHR(LogicalDevice, Swapchain, &ImageCount, SwapchainImages.data());

		// Keep BufferCount truthful in case the driver handed back a different count than what we requested.
		// This can happen on some platforms where the driver may choose to return a different number of images than requested.
		Desc.BufferCount = ImageCount;

		SwapchainImageViews.resize(SwapchainImages.size());
		for (size_t i = 0; i < SwapchainImages.size(); ++i) {
			VkImageViewCreateInfo ViewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
			ViewInfo.image = SwapchainImages[i];
			ViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			ViewInfo.format = SwapchainImageFormat;
			ViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			ViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			ViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			ViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			ViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			ViewInfo.subresourceRange.baseMipLevel = 0;
			ViewInfo.subresourceRange.levelCount = 1;
			ViewInfo.subresourceRange.baseArrayLayer = 0;
			ViewInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(LogicalDevice, &ViewInfo, nullptr, &SwapchainImageViews[i]) != VK_SUCCESS) {
				SORA_ENGINE_ERROR("Failed to create image view at index {0}!", i);
				return false;
			}

			RHITextureDesc TextureDesc{};
			TextureDesc.Width = SwapchainExtent.width;
			TextureDesc.Height = SwapchainExtent.height;
			TextureDesc.Format = static_cast<RHIFormat>(SwapchainImageFormat);
			TextureDesc.Usage = RHITextureUsage::ColorAttachment;

			auto BackBufferTexture = std::make_shared<VulkanTexture>(
				SwapchainImages[i],
				SwapchainImageViews[i],
				TextureDesc,
				SwapchainImageFormat
			);

			BackBufferTexture->SetCurrentLayout(VK_IMAGE_LAYOUT_UNDEFINED);

			BackBuffers.push_back(BackBufferTexture);
		}

		for (size_t i = 0; i < ImageAvailableSemaphores.size(); ++i) {
			if (ImageAvailableSemaphores[i] != VK_NULL_HANDLE) {
				vkDestroySemaphore(LogicalDevice, ImageAvailableSemaphores[i], nullptr);
				ImageAvailableSemaphores[i] = VK_NULL_HANDLE;
			}
			if (i < RenderFinishedSemaphores.size() && RenderFinishedSemaphores[i] != VK_NULL_HANDLE) {
				vkDestroySemaphore(LogicalDevice, RenderFinishedSemaphores[i], nullptr);
				RenderFinishedSemaphores[i] = VK_NULL_HANDLE;
			}
		}

		constexpr uint32_t MaxFramesInFlight = 2;

		VkSemaphoreCreateInfo SemaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		ImageAvailableSemaphores.resize(MaxFramesInFlight);
		RenderFinishedSemaphores.resize(MaxFramesInFlight);

		for (uint32_t i = 0; i < MaxFramesInFlight; ++i) {
            if (vkCreateSemaphore(LogicalDevice, &SemaphoreInfo, nullptr, &ImageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(LogicalDevice, &SemaphoreInfo, nullptr, &RenderFinishedSemaphores[i]) != VK_SUCCESS) {
                SORA_ENGINE_ERROR("Failed to create synchronization semaphores!");
                return false;
            }
        }

		return true;
	}

}
