#pragma once

#include "Runtime/RHI/Public/RHICommandList.h"
#include <vulkan/vulkan.h>
#include <initializer_list>
#include <memory>

namespace Sora::RHI
{

	class VulkanCommandList : public RHICommandList {
	public:
		VulkanCommandList(VkDevice InDevice, VkCommandPool InCommandPool);
		virtual ~VulkanCommandList() override = default;

		virtual void Begin() override;
		virtual void End() override;

		// Dynamic Rendering (Vulkan 1.3+)
		void BeginRendering(std::shared_ptr<RHITexture> InColorTexture, float InR = 0.0f, float InG = 0.0f, float InB = 0.0f, float InA = 1.0f);
		void EndRendering();

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

	private:
		VkDevice LogicalDevice{ VK_NULL_HANDLE };
		VkCommandPool CommandPool{ VK_NULL_HANDLE };
		VkCommandBuffer CommandBuffer{ VK_NULL_HANDLE };
	};

} // namespace Sora::RHI
