#pragma once

#include "Core/SECI/Subsystem/Public/SECISubsystem.h"
#include "Core/SECI/EventLayer/Public/SECIEvent.h"
#include "Core/Logging/Public/Log.h"
#include "Public/RHIDevice.h"
#include <memory>

namespace Sora::RHI
{

	class SORA_API RHISubsystem : public SECI::SECISubsystem {
	public:
		RHISubsystem() = default;
		virtual ~RHISubsystem() = default;

		void SetDevice(std::unique_ptr<RHIDevice> InDevice) {
			if (!Device) {
				Device = std::move(InDevice);
			} else {
				SORA_ENGINE_WARN("RHIDevice already exists.");
			}
		}

		virtual bool OnInitialize() override {
			// Initialize abstract device here (e.g., VulkanDevice)
			if (!Device) {
				SORA_ENGINE_ERROR("RHISubsystem failed to initialize: No RHIDevice set! You must set an RHIDevice.");
				return false;
			}

			return Device->Initialize();
		}

		virtual void OnUpdate(float deltaTime) override {
			// Frame rendering execution / command submission
		}

		virtual void OnShutdown() override {
			if (Device) {
				Device->WaitIdle();
				Device->Shutdown();
			}
        }

		virtual std::string GetName() const override { return "RHISubsystem"; }
        virtual SECI::SECISubsystemPriority GetPriority() const override { return SECI::SECISubsystemPriority::High; }

        RHIDevice* GetDevice() const { return Device.get(); }

	private:
		std::unique_ptr<RHIDevice> Device;
	};

} // namespace Sora::SECI
