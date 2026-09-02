#pragma once

#include "Runtime/RHI/Public/Pipeline/RHIShaderModule.h"
#include <vulkan/vulkan.h>

namespace Sora::RHI
{

	class VulkanShaderModule : public RHIShaderModule {
	public:
		VulkanShaderModule(VkDevice InLogicalDevice, const RHIShaderModuleDesc& InDesc);
		virtual ~VulkanShaderModule() override;

		virtual const RHIShaderModuleDesc& GetDesc() const override { return Desc; }
		virtual RHIShaderStage GetStage() const override { return Desc.Stage; }

		VkShaderModule GetVkShaderModule() const { return ShaderModule; }

	private:
		VkDevice LogicalDevice{ VK_NULL_HANDLE };
		VkShaderModule ShaderModule{ VK_NULL_HANDLE };
		RHIShaderModuleDesc Desc{};
	};

} // namespace Sora::RHI
