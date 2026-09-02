#pragma once

#include "RHIPipelineTypes.h"
#include "RHIPipelineLayout.h"
#include "Runtime/RHI/Public/RHITypes.h"
#include "RHIShaderModule.h"
#include "Core/Core.h"
#include <memory>
#include <vector>

namespace Sora::RHI
{

	struct RHIVertexAttribute {
		uint32_t Location{ 0 };
		uint32_t Binding{ 0 };
		RHIFormat Format{ RHIFormat::R32G32B32_FLOAT };
		uint32_t Offset{ 0 };
	};

	struct RHIVertexBinding {
		uint32_t Binding{ 0 };
		uint32_t Stride{ 0 };
		RHIVertexInputRate InputRate{ RHIVertexInputRate::PerVertex };
	};

	struct RHIVertexInputLayout {
		std::vector<RHIVertexBinding> Bindings;
		std::vector<RHIVertexAttribute> Attributes;
	};

	struct RHIGraphicsPipelineDesc {
		std::shared_ptr<RHIShaderModule> VertexShader{ nullptr };
		std::shared_ptr<RHIShaderModule> FragmentShader{ nullptr };

		RHIVertexInputLayout VertexLayout;
		RHIRasterizerState RasterizerState{};
		RHIDepthStencilState DepthStencilState{};

		// One entry per ColorAttachmentFormats entry below; leave empty for a single
		// default (non-blending) attachment. See RHIAttachmentBlendState for why this
		// is per-attachment rather than one pipeline-wide blend setting.
		std::vector<RHIAttachmentBlendState> AttachmentBlendStates;

		RHIPrimitiveTopology Topology{ RHIPrimitiveTopology::TriangleList };

		// Descriptor sets + push constants this pipeline's shaders expect to be bound.
		RHIPipelineLayoutDesc LayoutDesc;

		// Dynamic Rendering Target Formats
		std::vector<RHIFormat> ColorAttachmentFormats;
		RHIFormat DepthAttachmentFormat{ RHIFormat::Unknown };
	};

	class SORA_API RHIGraphicsPipeline {
	public:
		virtual ~RHIGraphicsPipeline() = default;

		virtual const RHIGraphicsPipelineDesc& GetDesc() const = 0;
	};

} // namespace Sora::RHI
