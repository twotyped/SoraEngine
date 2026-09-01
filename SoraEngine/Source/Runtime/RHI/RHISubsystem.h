#pragma once

#include "Core/SECI/Subsystem/Public/SECISubsystem.h"
#include "Core/SECI/EventLayer/Public/SECIEvent.h"
#include "Core/Logging/Public/Log.h"
#include "Public/RHIDevice.h"
#include "Public/RHISwapchain.h"
#include "Public/RHICommandList.h"
#include "Public/RHITexture.h"
#include <memory>

namespace Sora
{

	class SORA_API RHISubsystem : public SECI::SECISubsystem {
	public:
		explicit RHISubsystem(SECI::SECIEventBus* InEventBus, std::unique_ptr<RHI::RHIDevice> InDevice, void* InNativeWindowHandle)
			: EventBus(InEventBus), Device(std::move(InDevice)), NativeWindowHandle(InNativeWindowHandle) {}
		virtual ~RHISubsystem() = default;

		void SetDevice(std::unique_ptr<RHI::RHIDevice> InDevice) {
			if (!Device) {
				Device = std::move(InDevice);
			} else {
				SORA_ENGINE_WARN("RHIDevice already set on RHISubsystem.");
			}
		}

		void SetNativeWindowHandle(void* InNativeWindowHandle) {
			if (!NativeWindowHandle) {
				NativeWindowHandle = InNativeWindowHandle;
			}
			else {
				SORA_ENGINE_WARN("Native window handle already set on RHISubsystem.");
			}
		}

		virtual bool OnInitialize() override {
			if (IsInitialized()) {
				SORA_ENGINE_WARN("RHISubsystem is already initialized.");
				return true;
			}

			if (!Device) {
				SORA_ENGINE_ERROR("RHISubsystem failed to initialize: No RHIDevice set! You must set an RHIDevice.");
				return false;
			}

			if (!NativeWindowHandle) {
				SORA_ENGINE_ERROR("RHISubsystem failed to initialize: No native window handle set! You must set a native window handle.");
				return false;
			}

			SetInitialized(true);
			return Device->Initialize(NativeWindowHandle);
		}

		void OnUpdate(float InDeltaTime) {
			if (!Device) {
				return;
			}

			Device->BeginFrame();

			// Grab active swapchain texture & issue clear
			auto Swapchain = Device->GetSwapchain();
			if (Swapchain) {
				auto CurrentTexture = Swapchain->GetCurrentBackBuffer();
				auto CmdList = Device->GetMainCommandList();

				if (CurrentTexture && CmdList) {
					// Clear to a deep dark blue / charcoal color (R: 0.1, G: 0.1, B: 0.12, A: 1.0)
					CmdList->ClearColorTexture(CurrentTexture, 0.1f, 0.1f, 0.12f, 1.0f);
				}
			}

			Device->EndFrame();
		}

		virtual void OnShutdown() override {
			if (!IsInitialized()) {
				SORA_ENGINE_WARN("RHISubsystem is not initialized or already shut down.");
				return;
			}
			if (Device) {
				Device->WaitIdle();
				Device->Shutdown();
			}
			SetInitialized(false);
        }

		virtual std::string GetName() const override { return "RHISubsystem"; }
        virtual SECI::SECISubsystemPriority GetPriority() const override { return SECI::SECISubsystemPriority::High; }

        RHI::RHIDevice* GetDevice() const { return Device.get(); }

	private:
		SECI::SECIEventBus* EventBus{ nullptr };
		std::unique_ptr<RHI::RHIDevice> Device;
		void* NativeWindowHandle{ nullptr };
	};

} // namespace Sora::SECI
