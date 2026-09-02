#pragma once

#include "Core/Core.h"
#include <cstdint>

namespace Sora::RHI
{

	enum class RHIFormat {
		Unknown,

        // SDR
		R8G8B8A8_UNORM,		// 8-bit 4-byte unorm RGBA (typically Vulkan/OpenGL)
		B8G8R8A8_UNORM,		// 8-bit 4-byte unorm BGRA (typically DirectX)
		R32G32B32_FLOAT,	// 32-bit 12-byte float RGB
		R32G32B32A32_FLOAT,	// 32-bit 16-byte float RGBA

        // HDR
		A2R10G10B10_UNORM,	// 10-bit 4-byte unorm ARGB (PQ)
		R16G16B16A16_FLOAT,	// 16-bit 8-byte float RGBA (scRGB)
        
		// Depth formats
		D24_UNORM_S8_UINT,  // 24-bit 4-byte uint depth buffer (8-bit stencil buffer)
        D32_SFLOAT,			// 32-bit 4-byte float depth buffer (no stencil buffer)
	};

	enum class RHIBufferUsage : uint32_t {
        None         = 0,
        VertexBuffer = 1 << 0,
        IndexBuffer  = 1 << 1,
        UniformBuffer= 1 << 2,
        StorageBuffer= 1 << 3,
        TransferSrc  = 1 << 4,
        TransferDst  = 1 << 5
    };

    struct RHIBufferDesc {
        uint64_t Size{ 0 };
        RHIBufferUsage Usage{ RHIBufferUsage::None };
        bool CpuVisible{ false };
    };

    struct RHIViewport {
        float X{ 0.0f };
        float Y{ 0.0f };
        float Width{ 0.0f };
        float Height{ 0.0f };
        float MinDepth{ 0.0f };
        float MaxDepth{ 1.0f };
    };

    struct RHIRect2D {
        int32_t X{ 0 };
        int32_t Y{ 0 };
        uint32_t Width{ 0 };
        uint32_t Height{ 0 };
    };

	enum class ERHIPresentMode {
        Immediate, // Off (Vulkan: VK_PRESENT_MODE_IMMEDIATE_KHR)
        VSync,     // On (Vulkan: VK_PRESENT_MODE_FIFO_KHR)
        Mailbox    // Fast V-Sync / Triple Buffer (Vulkan: VK_PRESENT_MODE_MAILBOX_KHR)
    };

} // namespace Sora::RHI
