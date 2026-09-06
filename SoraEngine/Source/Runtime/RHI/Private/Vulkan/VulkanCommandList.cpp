#include "Runtime/RHI/Public/Vulkan/VulkanCommandList.h"
#include "Runtime/RHI/Public/Vulkan/VulkanBuffer.h"
#include "Runtime/RHI/Public/Vulkan/VulkanTexture.h"
#include "Runtime/RHI/Public/Vulkan/Pipeline/VulkanGraphicsPipeline.h"
#include "Core/Logging/Public/Log.h"

namespace Sora::RHI
{
	VulkanCommandList::VulkanCommandList(VulkanDevice* InDevice, VkCommandPool InCommandPool)
		: Device(InDevice), CommandPool(InCommandPool) 
	{
		LogicalDevice = Device->GetVkDevice();

		VkCommandBufferAllocateInfo AllocateInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		AllocateInfo.commandPool = CommandPool;
		AllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		AllocateInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(LogicalDevice, &AllocateInfo, &CommandBuffer) != VK_SUCCESS) {
			SORA_ENGINE_ERROR("Failed to allocate Vulkan Command Buffer!");
		}
	}

	VulkanCommandList::~VulkanCommandList() {
		if (CommandBuffer != VK_NULL_HANDLE && LogicalDevice != VK_NULL_HANDLE) {
			vkFreeCommandBuffers(LogicalDevice, CommandPool, 1, &CommandBuffer);
			CommandBuffer = VK_NULL_HANDLE;
		}
	}

	void VulkanCommandList::Begin() {
		// Reset the command buffer before starting new frame recording
		vkResetCommandBuffer(CommandBuffer, 0);

		VkCommandBufferBeginInfo BeginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		if (vkBeginCommandBuffer(CommandBuffer, &BeginInfo) != VK_SUCCESS) {
			SORA_ENGINE_ERROR("Failed to begin recording Vulkan Command Buffer!");
		}
	}

	void VulkanCommandList::End() {
		if (vkEndCommandBuffer(CommandBuffer) != VK_SUCCESS) {
			SORA_ENGINE_ERROR("Failed to end recording Vulkan Command Buffer!");
		}
	}

	void VulkanCommandList::Submit() {
		uint32_t CurrentFrame = Device->GetCurrentFrameIndex();
		VkFence FrameFence = Device->GetCurrentFrameFence();
		VkQueue GraphicsQueue = Device->GetGraphicsQueue();

		vkResetFences(LogicalDevice, 1, &FrameFence);
		VkSubmitInfo SubmitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		SubmitInfo.commandBufferCount = 1;
		SubmitInfo.pCommandBuffers = &CommandBuffer;

		VkSemaphore waitSemaphore = Device->GetCurrentImageAvailableSemaphore(); // Signaled by Swapchain Acquire
		VkSemaphore signalSemaphore = Device->GetCurrentRenderFinishedSemaphore(); // Signaled when rendering completes
		
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		SubmitInfo.waitSemaphoreCount = 1;
		SubmitInfo.pWaitSemaphores = &waitSemaphore;
		SubmitInfo.pWaitDstStageMask = waitStages;
		
		SubmitInfo.signalSemaphoreCount = 1;
		SubmitInfo.pSignalSemaphores = &signalSemaphore;

		if (vkQueueSubmit(GraphicsQueue, 1, &SubmitInfo, FrameFence) != VK_SUCCESS) {
			SORA_ENGINE_ERROR("VulkanCommandList Submit: Failed to queue graphics commands!");
		}
	}

	void VulkanCommandList::BeginRendering(const RHIRenderingInfo& InRenderingInfo) {
		std::vector<VkRenderingAttachmentInfo> ColorAttachments;
		ColorAttachments.reserve(InRenderingInfo.ColorAttachments.size());

		for (const auto& Attachment : InRenderingInfo.ColorAttachments) {
			auto VkTex = std::dynamic_pointer_cast<VulkanTexture>(Attachment.Texture);
			if (!VkTex) continue;

			VkRenderingAttachmentInfo AttachmentInfo{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
			AttachmentInfo.imageView = VkTex->GetVkImageView();
			AttachmentInfo.imageLayout = VkTex->GetCurrentLayout();
			AttachmentInfo.loadOp = Attachment.ClearOnLoad ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
			AttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			AttachmentInfo.clearValue.color.float32[0] = Attachment.ClearColor[0];
			AttachmentInfo.clearValue.color.float32[1] = Attachment.ClearColor[1];
			AttachmentInfo.clearValue.color.float32[2] = Attachment.ClearColor[2];
			AttachmentInfo.clearValue.color.float32[3] = Attachment.ClearColor[3];

			ColorAttachments.push_back(AttachmentInfo);
		}

		VkRenderingAttachmentInfo DepthAttachmentInfo{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
		bool bHasDepth = InRenderingInfo.HasDepthAttachment;
		if (bHasDepth) {
			auto VkDepthTex = std::dynamic_pointer_cast<VulkanTexture>(InRenderingInfo.DepthAttachment.Texture);
			if (VkDepthTex) {
				DepthAttachmentInfo.imageView = VkDepthTex->GetVkImageView();
				DepthAttachmentInfo.imageLayout = VkDepthTex->GetCurrentLayout();
				DepthAttachmentInfo.loadOp = InRenderingInfo.DepthAttachment.ClearOnLoad ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
				DepthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				DepthAttachmentInfo.clearValue.depthStencil.depth = InRenderingInfo.DepthAttachment.ClearColor[0];
				DepthAttachmentInfo.clearValue.depthStencil.stencil = 0;
			} else {
				bHasDepth = false;
			}
		}

		VkRenderingInfo VkRenderInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO };
		VkRenderInfo.renderArea.offset = { InRenderingInfo.RenderArea.X, InRenderingInfo.RenderArea.Y };
		VkRenderInfo.renderArea.extent = { InRenderingInfo.RenderArea.Width, InRenderingInfo.RenderArea.Height };
		VkRenderInfo.layerCount = 1;
		VkRenderInfo.colorAttachmentCount = static_cast<uint32_t>(ColorAttachments.size());
		VkRenderInfo.pColorAttachments = ColorAttachments.data();
		VkRenderInfo.pDepthAttachment = bHasDepth ? &DepthAttachmentInfo : nullptr;

		vkCmdBeginRendering(CommandBuffer, &VkRenderInfo);
	}

	void VulkanCommandList::EndRendering() {
		vkCmdEndRendering(CommandBuffer);
	}

	void VulkanCommandList::BindPipeline(std::shared_ptr<RHIGraphicsPipeline> InPipeline) {
		auto VkPipe = std::dynamic_pointer_cast<VulkanGraphicsPipeline>(InPipeline);
		if (!VkPipe) {
			SORA_ENGINE_ERROR("BindPipeline failed: pipeline is not a valid VulkanGraphicsPipeline!");
			return;
		}

		vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, VkPipe->GetVkPipeline());
		CurrentPipelineLayout = VkPipe->GetVkPipelineLayout();
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
			ImageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
			ImageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
			ImageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;

			// No queue family ownership transfer happening here — must be explicit,
			// since a zero-initialized index reads as "transfer to/from family 0".
			ImageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			ImageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			
			// Layout state transitions
			ImageBarrier.oldLayout = VkTex->GetCurrentLayout();

			VkImageLayout RealTrackedLayout = VkTex->GetCurrentLayout();
			if (Barrier.OldLayout == RHITextureLayout::Undefined && RealTrackedLayout != VK_IMAGE_LAYOUT_UNDEFINED) {
				ImageBarrier.oldLayout = RealTrackedLayout;
			} else {
				ImageBarrier.oldLayout = ConvertRHILayoutToVkLayout(Barrier.OldLayout);
			}

			ImageBarrier.newLayout = ConvertRHILayoutToVkLayout(Barrier.NewLayout);
			
			ImageBarrier.image = VkTex->GetVkImage();
			ImageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

			// Handle depth/stencil format aspect masks
			VkFormat Format = VkTex->GetVkFormat();
			if (Format == VK_FORMAT_D32_SFLOAT || Format == VK_FORMAT_D24_UNORM_S8_UINT || Format == VK_FORMAT_D16_UNORM) {
				ImageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
				if (Format == VK_FORMAT_D24_UNORM_S8_UINT) {
					ImageBarrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
				}
			}

			ImageBarrier.subresourceRange.baseMipLevel = 0;
			ImageBarrier.subresourceRange.levelCount = VkTex->GetDesc().MipLevels;
			ImageBarrier.subresourceRange.baseArrayLayer = 0;
			ImageBarrier.subresourceRange.layerCount = VkTex->GetDesc().ArrayLayers;

			// Update stored layout inside VulkanTexture
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

		// Note: Requires image to be in TRANSFER_DST_OPTIMAL or GENERAL layout
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
		if (VkTex->GetVkFormat() == VK_FORMAT_D24_UNORM_S8_UINT) {
			Range.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		Range.baseMipLevel = 0;
		Range.levelCount = VkTex->GetDesc().MipLevels;
		Range.baseArrayLayer = 0;
		Range.layerCount = VkTex->GetDesc().ArrayLayers;

		// Note: Requires image to be in TRANSFER_DST_OPTIMAL or GENERAL layout
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
			vkCmdBindIndexBuffer(CommandBuffer, VkBuf->GetVkBuffer(), static_cast<VkDeviceSize>(InOffset), VK_INDEX_TYPE_UINT32);
		}
	}

	void VulkanCommandList::Draw(uint32_t InVertexCount, uint32_t InInstanceCount, uint32_t InFirstVertex, uint32_t InFirstInstance) {
		vkCmdDraw(CommandBuffer, InVertexCount, InInstanceCount, InFirstVertex, InFirstInstance);
	}

	void VulkanCommandList::DrawIndexed(uint32_t InIndexCount, uint32_t InInstanceCount, uint32_t InFirstIndex, int32_t InVertexOffset, uint32_t InFirstInstance) {
		vkCmdDrawIndexed(CommandBuffer, InIndexCount, InInstanceCount, InFirstIndex, InVertexOffset, InFirstInstance);
	}

	void VulkanCommandList::BindUniformBuffer(uint32_t InSet, uint32_t InBinding, std::shared_ptr<RHIBuffer> InBuffer) {
		auto VkBuf = std::dynamic_pointer_cast<VulkanBuffer>(InBuffer);
		if (!VkBuf) {
			SORA_ENGINE_ERROR("BindUniformBuffer failed: InBuffer is null or not a valid VulkanBuffer!");
			return;
		}

		if (CurrentPipelineLayout == VK_NULL_HANDLE) {
			SORA_ENGINE_ERROR("BindUniformBuffer failed: No active pipeline layout! Call BindPipeline first.");
			return;
		}

		VkDescriptorBufferInfo BufferInfo{};
		BufferInfo.buffer = VkBuf->GetVkBuffer();
		BufferInfo.offset = 0;
		BufferInfo.range = VkBuf->GetDesc().Size;

		VkWriteDescriptorSet DescriptorWrite{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		DescriptorWrite.dstBinding = InBinding;
		DescriptorWrite.dstArrayElement = 0;
		DescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		DescriptorWrite.descriptorCount = 1;
		DescriptorWrite.pBufferInfo = &BufferInfo;

		// Load the push descriptor extension function directly from the device instance
		static auto pfnCmdPushDescriptorSetKHR = (PFN_vkCmdPushDescriptorSetKHR)vkGetDeviceProcAddr(LogicalDevice, "vkCmdPushDescriptorSetKHR");
		
		if (pfnCmdPushDescriptorSetKHR) {
			pfnCmdPushDescriptorSetKHR(
				CommandBuffer, 
				VK_PIPELINE_BIND_POINT_GRAPHICS, 
				CurrentPipelineLayout, 
				InSet, 
				1, 
				&DescriptorWrite
			);
		} else {
			SORA_ENGINE_ERROR("BindUniformBuffer failed: VK_KHR_push_descriptor extension function not found.");
		}
	}

	VkImageLayout VulkanCommandList::ConvertRHILayoutToVkLayout(RHITextureLayout InLayout) const {
		switch (InLayout) {
			case RHITextureLayout::Undefined:           return VK_IMAGE_LAYOUT_UNDEFINED;
			case RHITextureLayout::General:             return VK_IMAGE_LAYOUT_GENERAL;
			case RHITextureLayout::ColorAttachment:     return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			case RHITextureLayout::DepthStencilAttachment: return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			case RHITextureLayout::ShaderReadOnly:      return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			case RHITextureLayout::TransferSrc:         return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			case RHITextureLayout::TransferDst:         return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			case RHITextureLayout::Present:             return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			default:                                    return VK_IMAGE_LAYOUT_UNDEFINED;
		}
	}
} // namespace Sora::RHI
