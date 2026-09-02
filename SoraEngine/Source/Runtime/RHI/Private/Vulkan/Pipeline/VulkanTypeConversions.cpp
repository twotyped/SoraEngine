#include "Runtime/RHI/Public/Vulkan/Pipeline/VulkanTypeConversions.h"

namespace Sora::RHI
{

	VkFormat ToVkFormat(RHIFormat InFormat) {
		switch (InFormat) {
			case RHIFormat::R8G8B8A8_UNORM:		return VK_FORMAT_R8G8B8A8_UNORM;
			case RHIFormat::B8G8R8A8_UNORM:		return VK_FORMAT_B8G8R8A8_UNORM;
			case RHIFormat::R32G32B32_FLOAT:	return VK_FORMAT_R32G32B32_SFLOAT;
			case RHIFormat::R32G32B32A32_FLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
			case RHIFormat::D24_UNORM_S8_UINT:	return VK_FORMAT_D24_UNORM_S8_UINT;
			case RHIFormat::D32_SFLOAT:			return VK_FORMAT_D32_SFLOAT;
			default:							return VK_FORMAT_UNDEFINED;
		}
	}

	VkShaderStageFlagBits ToVkShaderStage(RHIShaderStage InStage) {
		switch (InStage) {
			case RHIShaderStage::Vertex:	return VK_SHADER_STAGE_VERTEX_BIT;
			case RHIShaderStage::Fragment:	return VK_SHADER_STAGE_FRAGMENT_BIT;
			case RHIShaderStage::Compute:	return VK_SHADER_STAGE_COMPUTE_BIT;
			default:						return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		}
	}

	VkShaderStageFlags ToVkShaderStageFlags(RHIShaderStage InFlags) {
		VkShaderStageFlags VkFlags = 0;
		if (HasShaderStage(InFlags, RHIShaderStage::Vertex))	VkFlags |= VK_SHADER_STAGE_VERTEX_BIT;
		if (HasShaderStage(InFlags, RHIShaderStage::Fragment))	VkFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
		if (HasShaderStage(InFlags, RHIShaderStage::Compute))	VkFlags |= VK_SHADER_STAGE_COMPUTE_BIT;
		return VkFlags;
	}

	VkPrimitiveTopology ToVkPrimitiveTopology(RHIPrimitiveTopology InTopology) {
		switch (InTopology) {
			case RHIPrimitiveTopology::PointList:		return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
			case RHIPrimitiveTopology::LineList:		return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			case RHIPrimitiveTopology::LineStrip:		return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
			case RHIPrimitiveTopology::TriangleList:	return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			case RHIPrimitiveTopology::TriangleStrip:	return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
			default:									return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
		}
	}

	VkCullModeFlags ToVkCullMode(RHICullMode InCullMode) {
		switch (InCullMode) {
			case RHICullMode::None:					return VK_CULL_MODE_NONE;
			case RHICullMode::Front:				return VK_CULL_MODE_FRONT_BIT;
			case RHICullMode::Back:					return VK_CULL_MODE_BACK_BIT;
			case RHICullMode::FrontAndBack:			return VK_CULL_MODE_FRONT_AND_BACK;
			default:								return VK_CULL_MODE_FLAG_BITS_MAX_ENUM;
		}
	}

	VkFrontFace ToVkFrontFace(RHIFrontFace InFrontFace) {
		switch (InFrontFace) {
			case RHIFrontFace::CounterClockwise:	return VK_FRONT_FACE_COUNTER_CLOCKWISE;
			case RHIFrontFace::Clockwise:			return VK_FRONT_FACE_CLOCKWISE;
			default:								return VK_FRONT_FACE_MAX_ENUM;
		}
	}

	VkPolygonMode ToVkPolygonMode(RHIFillMode InMode) {
		switch (InMode) {
			case RHIFillMode::Solid:				return VK_POLYGON_MODE_FILL;
			case RHIFillMode::Wireframe:			return VK_POLYGON_MODE_LINE;
			default:								return VK_POLYGON_MODE_MAX_ENUM;
		}
	}

	VkCompareOp ToVkCompareOp(RHICompareOp InOp) {
		switch (InOp) {
			case RHICompareOp::Never:				return VK_COMPARE_OP_NEVER;
			case RHICompareOp::Less:				return VK_COMPARE_OP_LESS;
			case RHICompareOp::Equal:				return VK_COMPARE_OP_EQUAL;
			case RHICompareOp::LessOrEqual:			return VK_COMPARE_OP_LESS_OR_EQUAL;
			case RHICompareOp::Greater:				return VK_COMPARE_OP_GREATER;
			case RHICompareOp::NotEqual:			return VK_COMPARE_OP_NOT_EQUAL;
			case RHICompareOp::GreaterOrEqual:		return VK_COMPARE_OP_GREATER_OR_EQUAL;
			case RHICompareOp::Always:				return VK_COMPARE_OP_ALWAYS;
			default:								return VK_COMPARE_OP_MAX_ENUM;
		}
	}

		VkBlendFactor ToVkBlendFactor(RHIBlendFactor InFactor) {
		switch (InFactor) {
			case RHIBlendFactor::Zero:             return VK_BLEND_FACTOR_ZERO;
			case RHIBlendFactor::One:              return VK_BLEND_FACTOR_ONE;
			case RHIBlendFactor::SrcColor:         return VK_BLEND_FACTOR_SRC_COLOR;
			case RHIBlendFactor::OneMinusSrcColor: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
			case RHIBlendFactor::DstColor:         return VK_BLEND_FACTOR_DST_COLOR;
			case RHIBlendFactor::OneMinusDstColor: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
			case RHIBlendFactor::SrcAlpha:         return VK_BLEND_FACTOR_SRC_ALPHA;
			case RHIBlendFactor::OneMinusSrcAlpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			case RHIBlendFactor::DstAlpha:         return VK_BLEND_FACTOR_DST_ALPHA;
			case RHIBlendFactor::OneMinusDstAlpha: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
			default:                               return VK_BLEND_FACTOR_ONE;
		}
	}
 
	VkBlendOp ToVkBlendOp(RHIBlendOp InOp) {
		switch (InOp) {
			case RHIBlendOp::Add:             return VK_BLEND_OP_ADD;
			case RHIBlendOp::Subtract:        return VK_BLEND_OP_SUBTRACT;
			case RHIBlendOp::ReverseSubtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
			case RHIBlendOp::Min:             return VK_BLEND_OP_MIN;
			case RHIBlendOp::Max:             return VK_BLEND_OP_MAX;
			default:                          return VK_BLEND_OP_ADD;
		}
	}
 
	VkColorComponentFlags ToVkColorComponentFlags(RHIColorWriteMask InMask) {
		VkColorComponentFlags VkFlags = 0;
		if ((InMask & RHIColorWriteMask::Red)   != RHIColorWriteMask::None) VkFlags |= VK_COLOR_COMPONENT_R_BIT;
		if ((InMask & RHIColorWriteMask::Green) != RHIColorWriteMask::None) VkFlags |= VK_COLOR_COMPONENT_G_BIT;
		if ((InMask & RHIColorWriteMask::Blue)  != RHIColorWriteMask::None) VkFlags |= VK_COLOR_COMPONENT_B_BIT;
		if ((InMask & RHIColorWriteMask::Alpha) != RHIColorWriteMask::None) VkFlags |= VK_COLOR_COMPONENT_A_BIT;
		return VkFlags;
	}
 
	VkDescriptorType ToVkDescriptorType(RHIDescriptorType InType) {
		switch (InType) {
			case RHIDescriptorType::UniformBuffer:        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			case RHIDescriptorType::StorageBuffer:        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			case RHIDescriptorType::SampledTexture:       return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			case RHIDescriptorType::StorageTexture:       return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			case RHIDescriptorType::Sampler:              return VK_DESCRIPTOR_TYPE_SAMPLER;
			case RHIDescriptorType::CombinedImageSampler: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			default:                                      return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		}
	}

} // namespace Sora::RHI