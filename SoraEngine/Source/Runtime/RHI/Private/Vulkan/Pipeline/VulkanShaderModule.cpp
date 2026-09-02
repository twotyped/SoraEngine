#include "Runtime/RHI/Public/Vulkan/Pipeline/VulkanShaderModule.h"
#include "Core/Logging/Public/Log.h"

namespace Sora::RHI
{

	VulkanShaderModule::VulkanShaderModule(VkDevice InLogicalDevice, const RHIShaderModuleDesc& InDesc)
		: LogicalDevice(InLogicalDevice), Desc(InDesc) {
		if (Desc.Bytecode.empty()) {
			SORA_ENGINE_ERROR("VulkanShaderModule: empty Bytecode (stage {0})!", static_cast<uint32_t>(Desc.Stage));
			return;
		}

		VkShaderModuleCreateInfo CreateInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		CreateInfo.codeSize = Desc.Bytecode.size() * sizeof(uint32_t);
		CreateInfo.pCode = Desc.Bytecode.data();

		if (vkCreateShaderModule(LogicalDevice, &CreateInfo, nullptr, &ShaderModule) != VK_SUCCESS) {
			SORA_ENGINE_ERROR("VulkanShaderModule: failed to create shader module (stage {0})!", static_cast<uint32_t>(Desc.Stage));
		}
	}

	VulkanShaderModule::~VulkanShaderModule() {
		if (ShaderModule != VK_NULL_HANDLE && LogicalDevice != VK_NULL_HANDLE) {
			vkDestroyShaderModule(LogicalDevice, ShaderModule, nullptr);
			ShaderModule = VK_NULL_HANDLE;
		}
	}

} // namespace Sora::RHI