#include "Runtime/RHI/Public/Vulkan/VulkanTexture.h"
#include "Core/Logging/Public/Log.h"

namespace Sora::RHI
{
	// Internally allocated texture (render targets, depth buffers, offscreen textures)
	VulkanTexture::VulkanTexture(
		VmaAllocator InAllocator,
		VkDevice InLogicalDevice,
		const RHITextureDesc& InDesc,
		VkImageUsageFlags InVkUsage
	) : Desc(InDesc), LogicalDevice(InLogicalDevice), Allocator(InAllocator), bIsExternalImage(false)
	{
		Format = ConvertRHIFormatToVkFormat(Desc.Format);

		VkImageCreateInfo ImageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		switch (Desc.Dimension) {
			case RHITextureDimension::Texture3D:  ImageInfo.imageType = VK_IMAGE_TYPE_3D; break;
			default:                              ImageInfo.imageType = VK_IMAGE_TYPE_2D; break;
		}

		if (Desc.Dimension == RHITextureDimension::TextureCube) {
			ImageInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		}

		ImageInfo.extent.width = Desc.Width;
		ImageInfo.extent.height = Desc.Height;
		ImageInfo.extent.depth = (Desc.Dimension == RHITextureDimension::Texture3D) ? Desc.Depth : 1;
		ImageInfo.mipLevels = Desc.MipLevels;
		ImageInfo.arrayLayers = Desc.ArrayLayers;
		ImageInfo.format = Format;
		ImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		ImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		ImageInfo.usage = InVkUsage;
		ImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		ImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

		// Fold RHITextureUsage into the Vulkan usage flags in case the caller
		// passed the default and relied on Desc.Usage to describe intent.
		const uint32_t DescUsage = static_cast<uint32_t>(Desc.Usage);
		if (DescUsage & static_cast<uint32_t>(RHITextureUsage::Sampled))                ImageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
		if (DescUsage & static_cast<uint32_t>(RHITextureUsage::Storage))                ImageInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
		if (DescUsage & static_cast<uint32_t>(RHITextureUsage::ColorAttachment))        ImageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if (DescUsage & static_cast<uint32_t>(RHITextureUsage::DepthStencilAttachment)) ImageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		if (DescUsage & static_cast<uint32_t>(RHITextureUsage::TransferSrc))            ImageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		if (DescUsage & static_cast<uint32_t>(RHITextureUsage::TransferDst))            ImageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		VmaAllocationCreateInfo AllocCreateInfo{};
		AllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

		if (vmaCreateImage(Allocator, &ImageInfo, &AllocCreateInfo, &Image, &Allocation, nullptr) != VK_SUCCESS) {
			SORA_ENGINE_ERROR("Failed to create Vulkan Image ({0}x{1})!", Desc.Width, Desc.Height);
			return;
		}

		CurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		CreateImageView();
	}

	// Externally managed image (swapchain back buffers) — we don't own the VkImage,
	// only (optionally) the view, and never allocate or free the underlying memory.
	VulkanTexture::VulkanTexture(
		VkImage InImage,
		VkImageView InImageView,
		const RHITextureDesc& InDesc,
		VkFormat InFormat
	) : Desc(InDesc), Image(InImage), ImageView(InImageView), Format(InFormat), bIsExternalImage(true)
	{
	}

	VulkanTexture::~VulkanTexture() {
		Cleanup();
	}

	void VulkanTexture::CreateImageView() {
		VkImageViewCreateInfo ViewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		ViewInfo.image = Image;

		switch (Desc.Dimension) {
			case RHITextureDimension::Texture3D:  ViewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D; break;
			case RHITextureDimension::TextureCube: ViewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE; break;
			default:                               ViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; break;
		}

		ViewInfo.format = Format;
		ViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		ViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		ViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		ViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		VkImageAspectFlags AspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		if (Format == VK_FORMAT_D32_SFLOAT || Format == VK_FORMAT_D24_UNORM_S8_UINT) {
			AspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			if (Format == VK_FORMAT_D24_UNORM_S8_UINT) {
				AspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}

		ViewInfo.subresourceRange.aspectMask = AspectMask;
		ViewInfo.subresourceRange.baseMipLevel = 0;
		ViewInfo.subresourceRange.levelCount = Desc.MipLevels;
		ViewInfo.subresourceRange.baseArrayLayer = 0;
		ViewInfo.subresourceRange.layerCount = Desc.ArrayLayers;

		if (vkCreateImageView(LogicalDevice, &ViewInfo, nullptr, &ImageView) != VK_SUCCESS) {
			SORA_ENGINE_ERROR("Failed to create Vulkan Image View!");
		}
	}

	void VulkanTexture::Cleanup() {
		// Swapchain-owned images/views are destroyed by VulkanSwapchain, not us.
		if (bIsExternalImage) {
			return;
		}

		if (ImageView != VK_NULL_HANDLE && LogicalDevice != VK_NULL_HANDLE) {
			vkDestroyImageView(LogicalDevice, ImageView, nullptr);
			ImageView = VK_NULL_HANDLE;
		}

		if (Image != VK_NULL_HANDLE && Allocator != VK_NULL_HANDLE) {
			vmaDestroyImage(Allocator, Image, Allocation);
			Image = VK_NULL_HANDLE;
			Allocation = VK_NULL_HANDLE;
		}
	}

	VkFormat VulkanTexture::ConvertRHIFormatToVkFormat(RHIFormat InFormat) const {
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
} // namespace Sora::RHI