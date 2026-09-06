#pragma once

#include "Runtime/RHI/Public/RHICommandList.h"
#include "Runtime/RHI/Public/Vulkan/VulkanDevice.h"
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

namespace Sora::RHI
{
	class VulkanCommandList : public RHICommandList {
		public:
			VulkanCommandList(VulkanDevice* InDevice, VkCommandPool InCommandPool);
			virtual ~VulkanCommandList() override;
	 
			virtual void Begin() override;
			virtual void End() override;

			virtual void Submit() override;
	 
			virtual void BeginRendering(const RHIRenderingInfo& InRenderingInfo) override;
			virtual void EndRendering() override;
	 
			virtual void BindPipeline(std::shared_ptr<RHIGraphicsPipeline> InPipeline) override;
	 
			virtual void SetViewport(const RHIViewport& InViewport) override;
			virtual void SetScissor(const RHIRect2D& InScissor) override;
	 
			virtual void PipelineBarrier(std::initializer_list<RHITextureBarrier> InTextureBarriers) override;
	 
			virtual void ClearColorTexture(std::shared_ptr<RHITexture> InTexture, float InR, float InG, float InB, float InA) override;
			virtual void ClearDepthTexture(std::shared_ptr<RHITexture> InTexture, float InDepth, uint32_t InStencil) override;
	 
			virtual void BindVertexBuffer(std::shared_ptr<RHIBuffer> InBuffer, uint64_t InOffset = 0) override;
			virtual void BindIndexBuffer(std::shared_ptr<RHIBuffer> InBuffer, uint64_t InOffset = 0) override;
	 
			virtual void Draw(uint32_t InVertexCount, uint32_t InInstanceCount = 1, uint32_t InFirstVertex = 0, uint32_t InFirstInstance = 0) override;
			virtual void DrawIndexed(uint32_t InIndexCount, uint32_t InInstanceCount = 1, uint32_t InFirstIndex = 0, int32_t InVertexOffset = 0, uint32_t InFirstInstance = 0) override;
	 
			VkCommandBuffer GetVkCommandBuffer() const { return CommandBuffer; }

			virtual void BindUniformBuffer(uint32_t InSet, uint32_t InBinding, std::shared_ptr<RHIBuffer> InBuffer) override;
	 
		private:
			VkImageLayout ConvertRHILayoutToVkLayout(RHITextureLayout InLayout) const;
	 
		private:
			VulkanDevice* Device{ nullptr };
			VkDevice LogicalDevice{ VK_NULL_HANDLE };
			VkCommandPool CommandPool{ VK_NULL_HANDLE };
			VkCommandBuffer CommandBuffer{ VK_NULL_HANDLE };
	 
			// Layout of the pipeline bound by the most recent BindPipeline() call — needed
			// once descriptor set / push constant binding is added (vkCmdBindDescriptorSets
			// and vkCmdPushConstants both take a VkPipelineLayout, not a VkPipeline).
			VkPipelineLayout CurrentPipelineLayout{ VK_NULL_HANDLE };
		};
} // namespace Sora::RHI
