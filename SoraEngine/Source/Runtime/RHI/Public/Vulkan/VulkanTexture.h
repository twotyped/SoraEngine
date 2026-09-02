#pragma once

#include "Runtime/RHI/Public/RHITexture.h"
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

namespace Sora::RHI
{
	class VulkanTexture : public RHITexture {
	public:
		// Constructor for internally allocated textures (Depth buffers, Render Targets, Offscreen Textures)
		VulkanTexture(
			VmaAllocator InAllocator,
			VkDevice InLogicalDevice,
			const RHITextureDesc& InDesc,
			VkImageUsageFlags InVkUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		);

		// Constructor for externally managed VkImages (Swapchain BackBuffers)
		VulkanTexture(
			VkImage InImage,
			VkImageView InImageView,
			const RHITextureDesc& InDesc,
			VkFormat InFormat
		);

		virtual ~VulkanTexture() override;

		// RHITexture Overrides
		virtual const RHITextureDesc& GetDesc() const override { return Desc; }
		virtual uint32_t GetWidth() const override { return Desc.Width; }
		virtual uint32_t GetHeight() const override { return Desc.Height; }
		virtual RHIFormat GetFormat() const override { return Desc.Format; }

		// Vulkan-Specific Getters
		VkImage GetVkImage() const { return Image; }
		VkImageView GetVkImageView() const { return ImageView; }
		VkFormat GetVkFormat() const { return Format; }
		VkImageLayout GetCurrentLayout() const { return CurrentLayout; }

		// Layout State Tracking
		void SetCurrentLayout(VkImageLayout InLayout) { CurrentLayout = InLayout; }

	private:
		void CreateImageView();
		void Cleanup();

		VkFormat ConvertRHIFormatToVkFormat(RHIFormat InFormat) const;

	private:
		RHITextureDesc Desc{};

		VkDevice LogicalDevice{ VK_NULL_HANDLE };
		VmaAllocator Allocator{ VK_NULL_HANDLE };

		VkImage Image{ VK_NULL_HANDLE };
		VkImageView ImageView{ VK_NULL_HANDLE };
		VmaAllocation Allocation{ VK_NULL_HANDLE };

		VkFormat Format{ VK_FORMAT_UNDEFINED };
		VkImageLayout CurrentLayout{ VK_IMAGE_LAYOUT_UNDEFINED };

		bool bIsExternalImage{ false };
	};
} // namespace Sora::RHI
