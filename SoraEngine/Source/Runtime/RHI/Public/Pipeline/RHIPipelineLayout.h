#pragma once

#include "RHIShaderModule.h"
#include <cstdint>
#include <vector>

namespace Sora::RHI
{

	enum class RHIDescriptorType {
		UniformBuffer,
		StorageBuffer,
		SampledTexture,
		StorageTexture,
		Sampler,
		CombinedImageSampler,
	};

	struct RHIDescriptorBinding {
		uint32_t Binding{ 0 };
		RHIDescriptorType Type{ RHIDescriptorType::UniformBuffer };
		uint32_t Count{ 1 }; // > 1 for arrays of resources (e.g. a bindless texture array)
		RHIShaderStage VisibleStages{ RHIShaderStage::Vertex | RHIShaderStage::Fragment };
	};

	struct RHIDescriptorSetLayoutDesc {
		std::vector<RHIDescriptorBinding> Bindings;
	};

	struct RHIPushConstantRange {
		RHIShaderStage VisibleStages{ RHIShaderStage::Vertex | RHIShaderStage::Fragment };
		uint32_t Offset{ 0 };
		uint32_t Size{ 0 };
	};

	// Everything a VkPipelineLayout needs: which descriptor sets exist (index in the
	// vector == Vulkan set number) and what push constant ranges the pipeline uses.
	struct RHIPipelineLayoutDesc {
		std::vector<RHIDescriptorSetLayoutDesc> DescriptorSets;
		std::vector<RHIPushConstantRange> PushConstantRanges;
	};

} // namespace Sora::RHI
