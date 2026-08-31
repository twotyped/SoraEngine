#include "Runtime/RHI/Public/Devices/Vulkan/VulkanCommandList.h"
#include "Runtime/RHI/Public/Devices/Vulkan/VulkanBuffer.h"
#include "Runtime/RHI/Public/Devices/Vulkan/VulkanTexture.h"
#include "Core/Logging/Public/Log.h"

namespace Sora::RHI
{

	VulkanCommandList::VulkanCommandList(VkDevice InDevice, VkCommandPool InCommandPool)
		: LogicalDevice(InDevice), CommandPool(InCommandPool) {
		VkCommandBufferAllocateInfo AllocateInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		AllocateInfo.commandPool = CommandPool;
		AllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		AllocateInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(LogicalDevice, &AllocateInfo, &CommandBuffer) != VK_SUCCESS) {
			SORA_ENGINE_ERROR("Failed to allocate Vulkan Command Buffer!");
		}
	}

	void VulkanCommandList::Begin() {
		VkCommandBufferBeginInfo BeginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(CommandBuffer, &BeginInfo);
	}

	void VulkanCommandList::End() {
		vkEndCommandBuffer(CommandBuffer);
	}

	void VulkanCommandList::SetViewport(const RHIViewport& InViewport) {
		VkViewport Viewport{};
		Viewport.x = InViewport.X;
		Viewport.y = InViewport.Y;
		Viewport.width = InViewport.Width;
		Viewport.height = InViewport.Height;
		Viewport.minDepth = InViewport.MinDepth;
		Viewport.maxDepth = InViewport.MaxDepth;

		vkCmdSetViewport(CommandBuffer, 0, 1, &Viewport);
	}

	void VulkanCommandList::SetScissor(const RHIRect2D& InScissor) {
		VkRect2D Scissor{};
		Scissor.offset = { InScissor.X, InScissor.Y };
		Scissor.extent = { InScissor.Width, InScissor.Height };

		vkCmdSetScissor(CommandBuffer, 0, 1, &Scissor);
	}

	void VulkanCommandList::PipelineBarrier(std::initializer_list<RHITextureBarrier> InTextureBarriers) {
		std::vector<VkImageMemoryBarrier2> VkBarriers;
		VkBarriers.reserve(InTextureBarriers.size());

		for (const auto& Barrier : InTextureBarriers) {
			auto VkTex = std::dynamic_pointer_cast<VulkanTexture>(Barrier.Texture);
			if (!VkTex) continue;

			VkImageMemoryBarrier2 ImageBarrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
			ImageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
			ImageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
			ImageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
			ImageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
			
			// Translate layouts
			ImageBarrier.oldLayout = VkTex->GetCurrentLayout();
			// Convert RHITextureLayout to VkImageLayout here...
			ImageBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; 
			
			ImageBarrier.image = VkTex->GetVkImage();
			ImageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			ImageBarrier.subresourceRange.levelCount = 1;
			ImageBarrier.subresourceRange.layerCount = 1;

			VkTex->SetCurrentLayout(ImageBarrier.newLayout);
			VkBarriers.push_back(ImageBarrier);
		}

		VkDependencyInfo DependencyInfo{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
		DependencyInfo.imageMemoryBarrierCount = static_cast<uint32_t>(VkBarriers.size());
		DependencyInfo.pImageMemoryBarriers = VkBarriers.data();

		vkCmdPipelineBarrier2(CommandBuffer, &DependencyInfo);
	}

	void VulkanCommandList::ClearColorTexture(std::shared_ptr<RHITexture> InTexture, float InR, float InG, float InB, float InA) {
		auto VkTex = std::dynamic_pointer_cast<VulkanTexture>(InTexture);
		if (!VkTex) {
			SORA_ENGINE_ERROR("ClearColorTexture failed: InTexture is not a valid VulkanTexture!");
			return;
		}

		VkClearColorValue ClearValue{};
		ClearValue.float32[0] = InR;
		ClearValue.float32[1] = InG;
		ClearValue.float32[2] = InB;
		ClearValue.float32[3] = InA;

		VkImageSubresourceRange Range{};
		Range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		Range.baseMipLevel = 0;
		Range.levelCount = VkTex->GetDesc().MipLevels;
		Range.baseArrayLayer = 0;
		Range.layerCount = VkTex->GetDesc().ArrayLayers;

		// Requires the image to be in VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL or VK_IMAGE_LAYOUT_GENERAL
		vkCmdClearColorImage(CommandBuffer, VkTex->GetVkImage(), VkTex->GetCurrentLayout(), &ClearValue, 1, &Range);
	}

	void VulkanCommandList::ClearDepthTexture(std::shared_ptr<RHITexture> InTexture, float InDepth, uint32_t InStencil) {
		auto VkTex = std::dynamic_pointer_cast<VulkanTexture>(InTexture);
		if (!VkTex) {
			SORA_ENGINE_ERROR("ClearDepthTexture failed: InTexture is not a valid VulkanTexture!");
			return;
		}

		VkClearDepthStencilValue ClearValue{};
		ClearValue.depth = InDepth;
		ClearValue.stencil = InStencil;

		VkImageSubresourceRange Range{};
		Range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (VkTex->GetDesc().Format == RHIFormat::D24_UNORM_S8_UINT) {
			Range.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		Range.baseMipLevel = 0;
		Range.levelCount = VkTex->GetDesc().MipLevels;
		Range.baseArrayLayer = 0;
		Range.layerCount = VkTex->GetDesc().ArrayLayers;

		// Requires the image to be in VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL or VK_IMAGE_LAYOUT_GENERAL
		vkCmdClearDepthStencilImage(CommandBuffer, VkTex->GetVkImage(), VkTex->GetCurrentLayout(), &ClearValue, 1, &Range);
	}

	void VulkanCommandList::BindVertexBuffer(std::shared_ptr<RHIBuffer> InBuffer, uint64_t InOffset) {
		if (auto VkBuf = std::dynamic_pointer_cast<VulkanBuffer>(InBuffer)) {
			VkBuffer BufferHandle = VkBuf->GetVkBuffer();
			VkDeviceSize OffsetSize = static_cast<VkDeviceSize>(InOffset);
			vkCmdBindVertexBuffers(CommandBuffer, 0, 1, &BufferHandle, &OffsetSize);
		}
	}

	void VulkanCommandList::BindIndexBuffer(std::shared_ptr<RHIBuffer> InBuffer, uint64_t InOffset) {
		if (auto VkBuf = std::dynamic_pointer_cast<VulkanBuffer>(InBuffer)) {
			// Assumes 32-bit uint indices by default
			vkCmdBindIndexBuffer(CommandBuffer, VkBuf->GetVkBuffer(), static_cast<VkDeviceSize>(InOffset), VK_INDEX_TYPE_UINT32);
		}
	}

	void VulkanCommandList::Draw(uint32_t InVertexCount, uint32_t InInstanceCount, uint32_t InFirstVertex, uint32_t InFirstInstance) {
		vkCmdDraw(CommandBuffer, InVertexCount, InInstanceCount, InFirstVertex, InFirstInstance);
	}

	void VulkanCommandList::DrawIndexed(uint32_t InIndexCount, uint32_t InInstanceCount, uint32_t InFirstIndex, int32_t InVertexOffset, uint32_t InFirstInstance) {
		vkCmdDrawIndexed(CommandBuffer, InIndexCount, InInstanceCount, InFirstIndex, InVertexOffset, InFirstInstance);
	}

} // namespace Sora::RHI
