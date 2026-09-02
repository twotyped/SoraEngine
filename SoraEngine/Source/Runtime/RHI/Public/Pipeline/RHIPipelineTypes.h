#pragma once

#include <cstdint>

namespace Sora::RHI
{

	enum class RHIVertexInputRate {
		PerVertex,
		PerInstance
	};

	enum class RHIPrimitiveTopology {
        TriangleList,
        TriangleStrip,
        LineList,
        LineStrip,
        PointList
    };

    enum class RHICullMode {
        None,
        Front,
        Back,
        FrontAndBack
    };

    enum class RHIFillMode {
        Solid,
        Wireframe
    };

    enum class RHIFrontFace {
        CounterClockwise,
        Clockwise
    };

    enum class RHICompareOp {
        Never,
        Less,
        Equal,
        LessOrEqual,
        Greater,
        NotEqual,
        GreaterOrEqual,
        Always
    };

    struct RHIRasterizerState {
        RHICullMode CullMode{ RHICullMode::Back };
        RHIFillMode FillMode{ RHIFillMode::Solid };
        RHIFrontFace FrontFace{ RHIFrontFace::CounterClockwise };
        bool DepthClampEnable{ false };
        bool RasterizerDiscardEnable{ false };
    };

    struct RHIDepthStencilState {
        bool DepthTestEnable{ true };
        bool DepthWriteEnable{ true };
        RHICompareOp DepthCompareOp{ RHICompareOp::Less };
    };

    enum class RHIBlendFactor {
        Zero,
        One,
        SrcColor,
        OneMinusSrcColor,
        DstColor,
        OneMinusDstColor,
        SrcAlpha,
        OneMinusSrcAlpha,
        DstAlpha,
        OneMinusDstAlpha,
    };

    enum class RHIBlendOp {
        Add,
        Subtract,
        ReverseSubtract,
        Min,
        Max,
    };

    enum class RHIColorWriteMask : uint32_t {
        None  = 0,
        Red   = 1u << 0,
        Green = 1u << 1,
        Blue  = 1u << 2,
        Alpha = 1u << 3,
    };

    constexpr RHIColorWriteMask operator|(RHIColorWriteMask InLHS, RHIColorWriteMask InRHS) {
        return static_cast<RHIColorWriteMask>(static_cast<uint32_t>(InLHS) | static_cast<uint32_t>(InRHS));
    }

    constexpr RHIColorWriteMask operator&(RHIColorWriteMask InLHS, RHIColorWriteMask InRHS) {
        return static_cast<RHIColorWriteMask>(static_cast<uint32_t>(InLHS) & static_cast<uint32_t>(InRHS));
    }

    // Per color-attachment blend state — Vulkan attaches one of these to each render
    // target individually (VkPipelineColorBlendAttachmentState), rather than one
    // pipeline-wide setting. A pipeline's blend array should have one entry per
    // ColorAttachmentFormats entry in RHIGraphicsPipelineDesc; leave the vector empty
    // to fall back to a single default (non-blending) attachment.
    struct RHIAttachmentBlendState {
        bool BlendEnable{ false };

        RHIBlendFactor SrcColorBlendFactor{ RHIBlendFactor::SrcAlpha };
        RHIBlendFactor DstColorBlendFactor{ RHIBlendFactor::OneMinusSrcAlpha };
        RHIBlendOp ColorBlendOp{ RHIBlendOp::Add };

        RHIBlendFactor SrcAlphaBlendFactor{ RHIBlendFactor::One };
        RHIBlendFactor DstAlphaBlendFactor{ RHIBlendFactor::Zero };
        RHIBlendOp AlphaBlendOp{ RHIBlendOp::Add };

        RHIColorWriteMask ColorWriteMask{ RHIColorWriteMask::Red | RHIColorWriteMask::Green | RHIColorWriteMask::Blue | RHIColorWriteMask::Alpha };
    };

} // namespace Sora::RHI
