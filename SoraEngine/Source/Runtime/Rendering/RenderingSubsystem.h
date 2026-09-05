#pragma once

#include "RenderingTypes.h"
#include "Core/SECI/Subsystem/Public/SECISubsystem.h"
#include "Core/SECI/EventLayer/Public/SECIEventBus.h"
#include "Runtime/RHI/Public/RHIDevice.h"
#include "Public/Camera.h"
#include "Core/Logging/Public/Log.h"

namespace Sora
{

	class RenderingSubsystem : public SECI::SECISubsystem {
	public:
		explicit RenderingSubsystem(SECI::SECIEventBus* InEventBus, std::shared_ptr<RHI::RHIDevice> InRhiDevice, Rendering::Camera* InCamera)
			: EventBus(InEventBus), RhiDevice(InRhiDevice), Camera(InCamera) {}

		virtual ~RenderingSubsystem() override = default;

		inline virtual bool OnInitialize() override {
			if (!RhiDevice) {
				SORA_ENGINE_ERROR("RenderingSubsystem: Failed to locate operational RHIDevice!");
				return false;
			}

			// Allocate global uniform buffer for scene data (Camera V/P matrices, etc.)
			// Create global pipelines, render passes, and default material descriptors here

			SORA_ENGINE_INFO("RenderSubsystem Initialized successfully.");
			return true;
		}

		inline virtual void OnShutdown() override {
			RhiDevice->WaitIdle();
			Camera = nullptr;
		}

		inline virtual void OnUpdate(float deltaTime) override {
			if (!RhiDevice) return;
			if (!Camera) return;

			Camera->UpdateUBO();

			// pipeline here or something
		}

	private:
		SECI::SECIEventBus* EventBus{ nullptr };
		std::shared_ptr<RHI::RHIDevice> RhiDevice{ nullptr };
		Rendering::Camera* Camera{ nullptr };

	};

}
