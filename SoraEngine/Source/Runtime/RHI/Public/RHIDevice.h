#pragma once

#include "RHITypes.h"
#include "Pipeline/RHIShaderModule.h"
#include "Pipeline/RHIPipeline.h"
#include "RHITexture.h"
#include <memory>
#include <string>

namespace Sora::RHI
{
	class RHIBuffer;
    class RHITexture;
    class RHISwapchain;
    class RHICommandList;

    struct RHIDeviceCapabilities {
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
		virtual std::shared_ptr<RHITexture> CreateTexture(const RHITextureDesc& InDesc) = 0;
        virtual std::shared_ptr<RHISwapchain> CreateSwapchain(void* windowHandle, uint32_t width, uint32_t height) = 0;
        virtual std::shared_ptr<RHICommandList> CreateCommandList() = 0;

        virtual std::shared_ptr<RHIShaderModule> CreateShaderModule(const RHIShaderModuleDesc& desc) = 0;
		virtual std::shared_ptr<RHIGraphicsPipeline> CreateGraphicsPipeline(const RHIGraphicsPipelineDesc& desc) = 0;

		virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;
        virtual void WaitIdle() = 0;

        std::shared_ptr<RHISwapchain> GetSwapchain() const { return Swapchain; }
		std::shared_ptr<RHICommandList> GetMainCommandList() const { return MainCommandList; }

    protected:
        std::shared_ptr<RHISwapchain> Swapchain{ nullptr };
        std::shared_ptr<RHICommandList> MainCommandList{ nullptr };
    };
} // namespace Sora::RHI
