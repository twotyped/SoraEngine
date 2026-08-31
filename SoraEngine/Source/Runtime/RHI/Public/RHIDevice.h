#pragma once

#include "RHITypes.h"
#include <memory>

namespace Sora::RHI
{
	class RHIBuffer;
    class RHITexture;
    class RHISwapchain;
    class RHICommandList;

    struct RHIDeviceCapabilities
    {
        std::string DeviceName;
        uint32_t ApiVersion{ 0 };
        uint64_t DedicatedVideoMemory{ 0 };
        bool SupportsRayTracing{ false };
        bool SupportsBindless{ false };
    };

    class SORA_API RHIDevice {
    public:
        virtual ~RHIDevice() = default;

        virtual bool Initialize(void* InNativeWindowHandle) = 0;
        virtual void Shutdown() = 0;

        virtual const RHIDeviceCapabilities& GetCapabilities() const = 0;

        virtual std::shared_ptr<RHIBuffer> CreateBuffer(const RHIBufferDesc& desc) = 0;
        virtual std::shared_ptr<RHISwapchain> CreateSwapchain(void* windowHandle, uint32_t width, uint32_t height) = 0;
        virtual std::shared_ptr<RHICommandList> CreateCommandList() = 0;

		virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;
        virtual void WaitIdle() = 0;
    };
} // namespace Sora::RHI
