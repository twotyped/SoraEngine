#pragma once

#include "Runtime/RHI/Public/Pipeline/RHIPipeline.h"
#include <vulkan/vulkan.h>
#include <vector>

namespace Sora::RHI
{
	class VulkanGraphicsPipeline : public RHIGraphicsPipeline {
	public:
		VulkanGraphicsPipeline(VkDevice InLogicalDevice, const RHIGraphicsPipelineDesc& InDesc);
		virtual ~VulkanGraphicsPipeline() override;

		const RHIGraphicsPipelineDesc& GetDesc() const override { return Desc; }

		VkPipeline GetVkPipeline() const { return Pipeline; }
		VkPipelineLayout GetVkPipelineLayout() const { return PipelineLayout; }
		const std::vector<VkDescriptorSetLayout>& GetVkDescriptorSetLayouts() const { return DescriptorSetLayouts; }

	private:
		bool CreateDescriptorSetLayouts();
		bool CreatePipelineLayout();
		bool CreatePipeline();
		void Cleanup();

	private:
		VkDevice LogicalDevice{ VK_NULL_HANDLE };
		RHIGraphicsPipelineDesc Desc;

		std::vector<VkDescriptorSetLayout> DescriptorSetLayouts;
		VkPipelineLayout PipelineLayout{ VK_NULL_HANDLE };
		VkPipeline Pipeline{ VK_NULL_HANDLE };
	};
}
