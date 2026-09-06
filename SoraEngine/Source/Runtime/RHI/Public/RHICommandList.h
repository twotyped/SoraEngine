#pragma once

#include "RHITypes.h"
#include "Runtime/RHI/Public/Pipeline/RHIPipeline.h"
#include <array>
#include <memory>
#include <initializer_list>

namespace Sora::RHI
{

    class RHIBuffer;
    class RHITexture;

    enum class RHITextureLayout {
        Undefined,
        General,
        ColorAttachment,
        DepthStencilAttachment,
        ShaderReadOnly,
        TransferSrc,
        TransferDst,
        Present
    };

    struct RHITextureBarrier {
        std::shared_ptr<RHITexture> Texture;
        RHITextureLayout OldLayout;
        RHITextureLayout NewLayout;
    };

    struct RHIRenderingAttachmentInfo {
		std::shared_ptr<RHITexture> Texture{ nullptr };
		bool ClearOnLoad{ true };
		std::array<float, 4> ClearColor{ 0.0f, 0.0f, 0.0f, 1.0f };
	};

	struct RHIRenderingInfo {
		RHIRect2D RenderArea{};
		std::vector<RHIRenderingAttachmentInfo> ColorAttachments;
		RHIRenderingAttachmentInfo DepthAttachment{};
		bool HasDepthAttachment{ false };
	};

	class SORA_API RHICommandList {
	public:
		virtual ~RHICommandList() = default;

		virtual void Begin() = 0;
		virtual void End() = 0;

		virtual void Submit() = 0;

		virtual void BeginRendering(const RHIRenderingInfo& renderingInfo) = 0;
		virtual void EndRendering() = 0;

		virtual void BindPipeline(std::shared_ptr<RHIGraphicsPipeline> pipeline) = 0;

		virtual void SetViewport(const RHIViewport& viewport) = 0;
		virtual void SetScissor(const RHIRect2D& scissor) = 0;

		virtual void PipelineBarrier(std::initializer_list<RHITextureBarrier> textureBarriers) = 0;

		virtual void ClearColorTexture(std::shared_ptr<RHITexture> texture, float r, float g, float b, float a) = 0;
		virtual void ClearDepthTexture(std::shared_ptr<RHITexture> texture, float depth, uint32_t stencil) = 0;

		virtual void BindVertexBuffer(std::shared_ptr<RHIBuffer> buffer, uint64_t offset = 0) = 0;
		virtual void BindIndexBuffer(std::shared_ptr<RHIBuffer> buffer, uint64_t offset = 0) = 0;
		virtual void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) = 0;
		virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) = 0;

		virtual void BindUniformBuffer(uint32_t set, uint32_t binding, std::shared_ptr<RHIBuffer> buffer) = 0;
	};

} // namespace Sora::RHI
