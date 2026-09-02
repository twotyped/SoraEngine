#pragma once

#include "RHITypes.h"
#include <memory>

namespace Sora::RHI
{
    class RHITexture;

    struct RHISwapchainDesc {
        uint32_t Width{ 1280 };
        uint32_t Height{ 720 };
        uint32_t BufferCount{ 3 }; // Triple buffering by default
        bool VSync{ true };
		bool EnableHDR{ false };
        RHIFormat PreferredFormat{ RHIFormat::B8G8R8A8_UNORM };
    };

    class SORA_API RHISwapchain {
    public:
        virtual ~RHISwapchain() = default;

        virtual bool AcquireNextImage() = 0;
        virtual bool Present() = 0;
        virtual void Resize(uint32_t width, uint32_t height) = 0;

        virtual std::shared_ptr<RHITexture> GetCurrentBackBuffer() const = 0;
        virtual uint32_t GetCurrentImageIndex() const = 0;
        virtual const RHISwapchainDesc& GetDesc() const = 0;
    };

} // namespace Sora::RHI
