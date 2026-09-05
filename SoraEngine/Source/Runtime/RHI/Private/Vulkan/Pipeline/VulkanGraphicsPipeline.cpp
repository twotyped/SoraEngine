#include "Runtime/RHI/Public/Vulkan/Pipeline/VulkanGraphicsPipeline.h"
#include "Runtime/RHI/Public/Vulkan/Pipeline/VulkanShaderModule.h"
#include "Runtime/RHI/Public/Vulkan/Pipeline/VulkanTypeConversions.h"
#include "Core/Logging/Public/Log.h"
#include <algorithm>

namespace Sora::RHI
{

	VulkanGraphicsPipeline::VulkanGraphicsPipeline(VkDevice InLogicalDevice, const RHIGraphicsPipelineDesc& InDesc)
	: LogicalDevice(InLogicalDevice), Desc(InDesc) {
		if (!CreateDescriptorSetLayouts()) {
			SORA_ENGINE_ERROR("VulkanGraphicsPipeline: failed to create descriptor set layouts!");
			return;
		}
		if (!CreatePipelineLayout()) {
			SORA_ENGINE_ERROR("VulkanGraphicsPipeline: failed to create pipeline layout!");
			return;
		}
		if (!CreatePipeline()) {
			SORA_ENGINE_ERROR("VulkanGraphicsPipeline: failed to create graphics pipeline!");
		}
	}
 
	VulkanGraphicsPipeline::~VulkanGraphicsPipeline() {
		Cleanup();
	}
 
	bool VulkanGraphicsPipeline::CreateDescriptorSetLayouts() {
		DescriptorSetLayouts.reserve(Desc.LayoutDesc.DescriptorSets.size());
 
		for (const auto& SetDesc : Desc.LayoutDesc.DescriptorSets) {
			std::vector<VkDescriptorSetLayoutBinding> VkBindings;
			VkBindings.reserve(SetDesc.Bindings.size());
 
			for (const auto& Binding : SetDesc.Bindings) {
				VkDescriptorSetLayoutBinding VkBinding{};
				VkBinding.binding = Binding.Binding;
				VkBinding.descriptorType = ToVkDescriptorType(Binding.Type);
				VkBinding.descriptorCount = Binding.Count;
				VkBinding.stageFlags = ToVkShaderStageFlags(Binding.VisibleStages);
				VkBindings.push_back(VkBinding);
			}
 
			VkDescriptorSetLayoutCreateInfo LayoutInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
			LayoutInfo.bindingCount = static_cast<uint32_t>(VkBindings.size());
			LayoutInfo.pBindings = VkBindings.data();
 
			VkDescriptorSetLayout SetLayout{ VK_NULL_HANDLE };
			if (vkCreateDescriptorSetLayout(LogicalDevice, &LayoutInfo, nullptr, &SetLayout) != VK_SUCCESS) {
				SORA_ENGINE_ERROR("Failed to create Vulkan Descriptor Set Layout!");
				return false;
			}
			DescriptorSetLayouts.push_back(SetLayout);
		}
 
		return true;
	}
 
	bool VulkanGraphicsPipeline::CreatePipelineLayout() {
		std::vector<VkPushConstantRange> VkPushConstants;
		VkPushConstants.reserve(Desc.LayoutDesc.PushConstantRanges.size());
 
		for (const auto& Range : Desc.LayoutDesc.PushConstantRanges) {
			VkPushConstantRange VkRange{};
			VkRange.stageFlags = ToVkShaderStageFlags(Range.VisibleStages);
			VkRange.offset = Range.Offset;
			VkRange.size = Range.Size;
			VkPushConstants.push_back(VkRange);
		}
 
		VkPipelineLayoutCreateInfo LayoutInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		LayoutInfo.setLayoutCount = static_cast<uint32_t>(DescriptorSetLayouts.size());
		LayoutInfo.pSetLayouts = DescriptorSetLayouts.data();
		LayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(VkPushConstants.size());
		LayoutInfo.pPushConstantRanges = VkPushConstants.data();
 
		return vkCreatePipelineLayout(LogicalDevice, &LayoutInfo, nullptr, &PipelineLayout) == VK_SUCCESS;
	}
 
	bool VulkanGraphicsPipeline::CreatePipeline() {
		auto VkVertexShader = std::dynamic_pointer_cast<VulkanShaderModule>(Desc.VertexShader);
		auto VkFragmentShader = std::dynamic_pointer_cast<VulkanShaderModule>(Desc.FragmentShader);
 
		if (!VkVertexShader || !VkFragmentShader) {
			SORA_ENGINE_ERROR("VulkanGraphicsPipeline: vertex/fragment shader missing or not a VulkanShaderModule!");
			return false;
		}
 
		std::vector<VkPipelineShaderStageCreateInfo> ShaderStages;
 
		VkPipelineShaderStageCreateInfo VertStageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		VertStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		VertStageInfo.module = VkVertexShader->GetVkShaderModule();
		VertStageInfo.pName = VkVertexShader->GetDesc().EntryPoint.c_str();
		ShaderStages.push_back(VertStageInfo);
 
		VkPipelineShaderStageCreateInfo FragStageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		FragStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		FragStageInfo.module = VkFragmentShader->GetVkShaderModule();
		FragStageInfo.pName = VkFragmentShader->GetDesc().EntryPoint.c_str();
		ShaderStages.push_back(FragStageInfo);
 
		// --- Vertex input ---
		std::vector<VkVertexInputBindingDescription> VkBindings;
		for (const auto& Binding : Desc.VertexLayout.Bindings) {
			VkVertexInputBindingDescription VkBinding{};
			VkBinding.binding = Binding.Binding;
			VkBinding.stride = Binding.Stride;
			VkBinding.inputRate = (Binding.InputRate == RHIVertexInputRate::PerInstance)
				? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
			VkBindings.push_back(VkBinding);
		}
 
		std::vector<VkVertexInputAttributeDescription> VkAttributes;
		for (const auto& Attribute : Desc.VertexLayout.Attributes) {
			VkVertexInputAttributeDescription VkAttribute{};
			VkAttribute.location = Attribute.Location;
			VkAttribute.binding = Attribute.Binding;
			VkAttribute.format = ToVkFormat(Attribute.Format);
			VkAttribute.offset = Attribute.Offset;
			VkAttributes.push_back(VkAttribute);
		}
 
		VkPipelineVertexInputStateCreateInfo VertexInputState{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		VertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(VkBindings.size());
		VertexInputState.pVertexBindingDescriptions = VkBindings.data();
		VertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(VkAttributes.size());
		VertexInputState.pVertexAttributeDescriptions = VkAttributes.data();
 
		VkPipelineInputAssemblyStateCreateInfo InputAssembly{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		InputAssembly.topology = ToVkPrimitiveTopology(Desc.Topology);
		InputAssembly.primitiveRestartEnable = VK_FALSE;
 
		// Viewport/scissor are supplied dynamically per-frame (VulkanCommandList::SetViewport
		// / SetScissor via vkCmdSetViewport/vkCmdSetScissor), so only the counts matter here.
		VkPipelineViewportStateCreateInfo ViewportState{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		ViewportState.viewportCount = 1;
		ViewportState.scissorCount = 1;
 
		VkPipelineRasterizationStateCreateInfo RasterizationState{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
		RasterizationState.depthClampEnable = Desc.RasterizerState.DepthClampEnable ? VK_TRUE : VK_FALSE;
		RasterizationState.rasterizerDiscardEnable = Desc.RasterizerState.RasterizerDiscardEnable ? VK_TRUE : VK_FALSE;
		RasterizationState.polygonMode = ToVkPolygonMode(Desc.RasterizerState.FillMode);
		RasterizationState.cullMode = ToVkCullMode(Desc.RasterizerState.CullMode);
		RasterizationState.frontFace = ToVkFrontFace(Desc.RasterizerState.FrontFace);
		RasterizationState.lineWidth = 1.0f;
 
		VkPipelineMultisampleStateCreateInfo MultisampleState{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
		MultisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
 
		VkPipelineDepthStencilStateCreateInfo DepthStencilState{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
		DepthStencilState.depthTestEnable = Desc.DepthStencilState.DepthTestEnable ? VK_TRUE : VK_FALSE;
		DepthStencilState.depthWriteEnable = Desc.DepthStencilState.DepthWriteEnable ? VK_TRUE : VK_FALSE;
		DepthStencilState.depthCompareOp = ToVkCompareOp(Desc.DepthStencilState.DepthCompareOp);
		DepthStencilState.depthBoundsTestEnable = Desc.DepthStencilState.DepthBoundsTestEnable ? VK_TRUE : VK_FALSE;
		DepthStencilState.stencilTestEnable = Desc.DepthStencilState.StencilTestEnable ? VK_TRUE : VK_FALSE;
 
		// --- Blend state: one attachment entry per color attachment format. Fall back to
		// a single default (non-blending, RGBA write) attachment if none were specified. ---
		std::vector<VkPipelineColorBlendAttachmentState> VkBlendAttachments;
		size_t AttachmentCount = std::max<size_t>(Desc.ColorAttachmentFormats.size(), 1);
		for (size_t i = 0; i < AttachmentCount; ++i) {
			const RHIAttachmentBlendState BlendDesc = (i < Desc.AttachmentBlendStates.size())
				? Desc.AttachmentBlendStates[i] : RHIAttachmentBlendState{};
 
			VkPipelineColorBlendAttachmentState VkBlendAttachment{};
			VkBlendAttachment.blendEnable = BlendDesc.BlendEnable ? VK_TRUE : VK_FALSE;
			VkBlendAttachment.srcColorBlendFactor = ToVkBlendFactor(BlendDesc.SrcColorBlendFactor);
			VkBlendAttachment.dstColorBlendFactor = ToVkBlendFactor(BlendDesc.DstColorBlendFactor);
			VkBlendAttachment.colorBlendOp = ToVkBlendOp(BlendDesc.ColorBlendOp);
			VkBlendAttachment.srcAlphaBlendFactor = ToVkBlendFactor(BlendDesc.SrcAlphaBlendFactor);
			VkBlendAttachment.dstAlphaBlendFactor = ToVkBlendFactor(BlendDesc.DstAlphaBlendFactor);
			VkBlendAttachment.alphaBlendOp = ToVkBlendOp(BlendDesc.AlphaBlendOp);
			VkBlendAttachment.colorWriteMask = ToVkColorComponentFlags(BlendDesc.ColorWriteMask);
 
			VkBlendAttachments.push_back(VkBlendAttachment);
		}
 
		VkPipelineColorBlendStateCreateInfo ColorBlendState{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
		ColorBlendState.attachmentCount = static_cast<uint32_t>(VkBlendAttachments.size());
		ColorBlendState.pAttachments = VkBlendAttachments.data();
 
		std::vector<VkDynamicState> DynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo DynamicState{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
		DynamicState.dynamicStateCount = static_cast<uint32_t>(DynamicStates.size());
		DynamicState.pDynamicStates = DynamicStates.data();
 
		// --- Dynamic rendering: describe attachment formats directly, no VkRenderPass ---
		std::vector<VkFormat> VkColorFormats;
		VkColorFormats.reserve(Desc.ColorAttachmentFormats.size());
		for (RHIFormat Format : Desc.ColorAttachmentFormats) {
			VkColorFormats.push_back(ToVkFormat(Format));
		}
 
		VkPipelineRenderingCreateInfo RenderingInfo{ VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
		RenderingInfo.colorAttachmentCount = static_cast<uint32_t>(VkColorFormats.size());
		RenderingInfo.pColorAttachmentFormats = VkColorFormats.data();
		if (Desc.DepthAttachmentFormat != RHIFormat::Unknown) {
			RenderingInfo.depthAttachmentFormat = ToVkFormat(Desc.DepthAttachmentFormat);
		}
 
		VkGraphicsPipelineCreateInfo PipelineInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
		PipelineInfo.pNext = &RenderingInfo;
		PipelineInfo.stageCount = static_cast<uint32_t>(ShaderStages.size());
		PipelineInfo.pStages = ShaderStages.data();
		PipelineInfo.pVertexInputState = &VertexInputState;
		PipelineInfo.pInputAssemblyState = &InputAssembly;
		PipelineInfo.pViewportState = &ViewportState;
		PipelineInfo.pRasterizationState = &RasterizationState;
		PipelineInfo.pMultisampleState = &MultisampleState;
		PipelineInfo.pDepthStencilState = &DepthStencilState;
		PipelineInfo.pColorBlendState = &ColorBlendState;
		PipelineInfo.pDynamicState = &DynamicState;
		PipelineInfo.layout = PipelineLayout;
		PipelineInfo.renderPass = VK_NULL_HANDLE; // dynamic rendering — no render pass object
		PipelineInfo.subpass = 0;
 
		return vkCreateGraphicsPipelines(LogicalDevice, VK_NULL_HANDLE, 1, &PipelineInfo, nullptr, &Pipeline) == VK_SUCCESS;
	}
 
	void VulkanGraphicsPipeline::Cleanup() {
		if (Pipeline != VK_NULL_HANDLE && LogicalDevice != VK_NULL_HANDLE) {
			vkDestroyPipeline(LogicalDevice, Pipeline, nullptr);
			Pipeline = VK_NULL_HANDLE;
		}
		if (PipelineLayout != VK_NULL_HANDLE && LogicalDevice != VK_NULL_HANDLE) {
			vkDestroyPipelineLayout(LogicalDevice, PipelineLayout, nullptr);
			PipelineLayout = VK_NULL_HANDLE;
		}
		for (VkDescriptorSetLayout SetLayout : DescriptorSetLayouts) {
			if (SetLayout != VK_NULL_HANDLE && LogicalDevice != VK_NULL_HANDLE) {
				vkDestroyDescriptorSetLayout(LogicalDevice, SetLayout, nullptr);
			}
		}
		DescriptorSetLayouts.clear();
	}

} // namespace Sora::RHI
