#include "Runtime/RHI/Public/Devices/Vulkan/VulkanTexture.h"
#include "Core/Logging/Public/Log.h"

namespace Sora::RHI
{

	static VkFormat ConvertRHIFormatToVkFormat(RHIFormat InFormat) {
		switch (InFormat) {
			case RHIFormat::R8G8B8A8_UNORM:     return VK_FORMAT_R8G8B8A8_UNORM;
			case RHIFormat::B8G8R8A8_UNORM:     return VK_FORMAT_B8G8R8A8_UNORM;
			case RHIFormat::R32G32B32_FLOAT:    return VK_FORMAT_R32G32B32_SFLOAT;
			case RHIFormat::R32G32B32A32_FLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
			case RHIFormat::D24_UNORM_S8_UINT:  return VK_FORMAT_D24_UNORM_S8_UINT;
			case RHIFormat::D32_SFLOAT:         return VK_FORMAT_D32_SFLOAT;
			default:                            return VK_FORMAT_UNDEFINED;
		}
	}

	VulkanTexture::VulkanTexture(VkDevice InDevice, VmaAllocator InAllocator, const RHITextureDesc& InDesc)
		: LogicalDevice(InDevice), Allocator(InAllocator), Desc(InDesc), bIsOwning(true) {
		VkImageCreateInfo ImageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		ImageInfo.imageType = VK_IMAGE_TYPE_2D;
		ImageInfo.extent.width = InDesc.Width;
		ImageInfo.extent.height = InDesc.Height;
		ImageInfo.extent.depth = InDesc.Depth;
		ImageInfo.mipLevels = InDesc.MipLevels;
		ImageInfo.arrayLayers = InDesc.ArrayLayers;
		ImageInfo.format = ConvertRHIFormatToVkFormat(InDesc.Format);
		ImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		ImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		ImageInfo.usage = static_cast<VkImageUsageFlags>(InDesc.Usage);
		ImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		ImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo AllocInfo{};
		AllocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

		if (vmaCreateImage(Allocator, &ImageInfo, &AllocInfo, &Image, &Allocation, nullptr) != VK_SUCCESS) {
			SORA_ENGINE_ERROR("Failed to allocate Vulkan Image!");
			return;
		}

		VkImageViewCreateInfo ViewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		ViewInfo.image = Image;
		ViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ViewInfo.format = ImageInfo.format;
		ViewInfo.subresourceRange.aspectMask = (InDesc.Usage == RHITextureUsage::DepthStencilAttachment) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		ViewInfo.subresourceRange.baseMipLevel = 0;
		ViewInfo.subresourceRange.levelCount = InDesc.MipLevels;
		ViewInfo.subresourceRange.baseArrayLayer = 0;
		ViewInfo.subresourceRange.layerCount = InDesc.ArrayLayers;

		if (vkCreateImageView(LogicalDevice, &ViewInfo, nullptr, &ImageView) != VK_SUCCESS) {
			SORA_ENGINE_ERROR("Failed to create Vulkan Image View!");
		}
	}

	VulkanTexture::VulkanTexture(VkDevice InDevice, VkImage InImage, VkImageView InImageView, const RHITextureDesc& InDesc)
		: LogicalDevice(InDevice), Image(InImage), ImageView(InImageView), Desc(InDesc), bIsOwning(false) {
	}

	VulkanTexture::~VulkanTexture() {
		if (bIsOwning) {
			if (ImageView != VK_NULL_HANDLE) {
				vkDestroyImageView(LogicalDevice, ImageView, nullptr);
			}
			if (Image != VK_NULL_HANDLE) {
				vmaDestroyImage(Allocator, Image, Allocation);
			}
		}
	}

} // namespace Sora::RHI
