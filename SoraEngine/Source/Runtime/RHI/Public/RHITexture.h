#pragma once

#include "RHITypes.h"

namespace Sora::RHI
{

	enum class RHITextureDimension {
		Texture2D,
		Texture3D,
		TextureCube,
	};

	enum class RHITextureUsage : uint32_t {
        None               = 0,
        Sampled            = 1 << 0,
        Storage            = 1 << 1,
        ColorAttachment    = 1 << 2,
        DepthStencilAttachment = 1 << 3,
        TransferSrc        = 1 << 4,
        TransferDst        = 1 << 5
    };
      
	struct RHITextureDesc {
        uint32_t Width{ 1 };
        uint32_t Height{ 1 };
        uint32_t Depth{ 1 };
        uint32_t MipLevels{ 1 };
        uint32_t ArrayLayers{ 1 };
        RHIFormat Format{ RHIFormat::R8G8B8A8_UNORM };
        RHITextureDimension Dimension{ RHITextureDimension::Texture2D };
        RHITextureUsage Usage{ RHITextureUsage::None };
    };

    class SORA_API RHITexture {
    public:
        virtual ~RHITexture() = default;

        virtual const RHITextureDesc& GetDesc() const = 0;
        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
        virtual RHIFormat GetFormat() const = 0;
    };

} // namespace Sora::RHI
