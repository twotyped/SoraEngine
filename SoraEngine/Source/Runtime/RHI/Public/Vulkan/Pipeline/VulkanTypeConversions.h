#pragma once

#include "Runtime/RHI/Public/RHITypes.h"
#include "Runtime/RHI/Public/Pipeline/RHIPipelineTypes.h"
#include "Runtime/RHI/Public/Pipeline/RHIShaderModule.h"
#include "Runtime/RHI/Public/Pipeline/RHIPipelineLayout.h"
#include <vulkan/vulkan.h>

namespace Sora::RHI
{

	VkFormat ToVkFormat(RHIFormat InFormat);

	VkShaderStageFlagBits ToVkShaderStage(RHIShaderStage InStage);
	VkShaderStageFlags ToVkShaderStageFlags(RHIShaderStage InFlags);

	VkPrimitiveTopology ToVkPrimitiveTopology(RHIPrimitiveTopology InTopology);
	VkCullModeFlags ToVkCullMode(RHICullMode InCullMode);
	VkFrontFace ToVkFrontFace(RHIFrontFace InFrontFace);
	VkPolygonMode ToVkPolygonMode(RHIFillMode InMode);
	VkCompareOp ToVkCompareOp(RHICompareOp InOp);
 
	VkBlendFactor ToVkBlendFactor(RHIBlendFactor InFactor);
	VkBlendOp ToVkBlendOp(RHIBlendOp InOp);
	VkColorComponentFlags ToVkColorComponentFlags(RHIColorWriteMask InMask);
 
	VkDescriptorType ToVkDescriptorType(RHIDescriptorType InType);

} // namespace Sora::RHI
