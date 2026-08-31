#pragma once

#include "Runtime/RHI/Public/RHITexture.h"
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

namespace Sora::RHI
{

	class VulkanTexture : public RHITexture {
	public:
		// Standalone Texture creation (Managed by VMA)
		VulkanTexture(VkDevice InDevice, VmaAllocator InAllocator, const RHITextureDesc& InDesc);

		// Non-owning Texture wrapper (Used for Swapchain images)
		VulkanTexture(VkDevice InDevice, VkImage InImage, VkImageView InImageView, const RHITextureDesc& InDesc);

		virtual ~VulkanTexture() override;

		virtual const RHITextureDesc& GetDesc() const override { return Desc; }
		virtual uint32_t GetWidth() const override { return Desc.Width; }
		virtual uint32_t GetHeight() const override { return Desc.Height; }
		virtual RHIFormat GetFormat() const override { return Desc.Format; }

		VkImage GetVkImage() const { return Image; }
		VkImageView GetVkImageView() const { return ImageView; }
		VkImageLayout GetCurrentLayout() const { return CurrentLayout; }
		void SetCurrentLayout(VkImageLayout InLayout) { CurrentLayout = InLayout; }

	private:
		VkDevice LogicalDevice{ VK_NULL_HANDLE };
		VmaAllocator Allocator{ VK_NULL_HANDLE };

		VkImage Image{ VK_NULL_HANDLE };
		VkImageView ImageView{ VK_NULL_HANDLE };
		VmaAllocation Allocation{ VK_NULL_HANDLE };

		VkImageLayout CurrentLayout{ VK_IMAGE_LAYOUT_UNDEFINED };
		RHITextureDesc Desc{};
		bool bIsOwning{ true };
	};

} // namespace Sora::RHI
