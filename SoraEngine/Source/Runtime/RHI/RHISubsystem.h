#pragma once

#include "Core/SECI/Subsystem/Public/SECISubsystem.h"
#include "Core/SECI/EventLayer/Public/SECIEvent.h"
#include "Core/SECI/EventLayer/Public/SECIEventBus.h"
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
		explicit RHISubsystem(SECI::SECIEventBus* InEventBus, std::shared_ptr<RHI::RHIDevice> InDevice, void* InNativeWindowHandle)
			: EventBus(InEventBus), Device(InDevice), NativeWindowHandle(InNativeWindowHandle) {}
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
			if (!Device) {
				SORA_ENGINE_ERROR("RHISubsystem failed to initialize: No RHIDevice set!");
				return false;
			}

			if (!NativeWindowHandle) {
				SORA_ENGINE_ERROR("RHISubsystem failed to initialize: No native window handle set!");
				return false;
			}

			if (!Device->Initialize(NativeWindowHandle)) {
				SORA_ENGINE_ERROR("RHISubsystem failed to initialize: RHIDevice initialization failed!");
				return false;
			}

			auto Swapchain = Device->CreateSwapchain(NativeWindowHandle, 1280, 720);
			if (!Swapchain) {
				SORA_ENGINE_ERROR("RHISubsystem failed to initialize: Failed to create initial RHISwapchain!");
				return false;
			}

			return true;
		}

		void OnUpdate(float InDeltaTime) {
			if (!Device) return;

			Device->BeginFrame();

			auto Swapchain = Device->GetSwapchain();
			auto CmdList = Device->GetMainCommandList();

			if (Swapchain && CmdList) {
				auto CurrentTexture = Swapchain->GetCurrentBackBuffer();
				if (CurrentTexture) {
					// 1. Transition UNDEFINED/PRESENT -> TRANSFER_DST for Clear execution
					CmdList->PipelineBarrier({
						{ CurrentTexture, RHI::RHITextureLayout::Undefined, RHI::RHITextureLayout::TransferDst }
					});

					// 2. Perform Clear Operation
					CmdList->ClearColorTexture(CurrentTexture, 0.1f, 0.1f, 0.12f, 1.0f);

					// 3. Transition TRANSFER_DST -> PRESENT for Queue Submission
					CmdList->PipelineBarrier({
						{ CurrentTexture, RHI::RHITextureLayout::TransferDst, RHI::RHITextureLayout::Present }
					});
				}
			}

			Device->EndFrame();
		}

		virtual void OnShutdown() override {
			if (Device) {
				Device->WaitIdle();
				Device->Shutdown();
			}
        }

		virtual void OnEvent(SECI::SECIEvent& InEvent) override {
			SECI::SECIEventHandler Dispatcher(InEvent);

			Dispatcher.Dispatch<SECI::WindowResizeEvent>([this](SECI::WindowResizeEvent& e) {
				SORA_ENGINE_TRACE("RHISubsystem, Window Resized: {0}x{1}", e.GetWidth(), e.GetHeight());
				if (Device->GetSwapchain()) {
					Device->GetSwapchain()->Resize(e.GetWidth(), e.GetHeight());
				} else {
					SORA_ENGINE_ERROR("RHISubsystem, Device's Swapchain is null.");
					return false;
				}
				return false;
			});

		}

		virtual SECI::SECIEventCategory GetSubscribedEventCategories() const override {
			return SECI::EventCategoryWindow;
        }

		virtual std::string GetName() const override { return "RHISubsystem"; }
        virtual SECI::SECISubsystemPriority GetPriority() const override { return SECI::SECISubsystemPriority::High; }

        std::shared_ptr<RHI::RHIDevice> GetDevice() const { return Device; }

	private:
		SECI::SECIEventBus* EventBus{ nullptr };
		std::shared_ptr<RHI::RHIDevice> Device;
		void* NativeWindowHandle{ nullptr };
	};

} // namespace Sora::SECI
