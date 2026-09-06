#pragma once

#include "RenderingTypes.h"
#include "Core/SECI/Subsystem/Public/SECISubsystem.h"
#include "Core/SECI/EventLayer/Public/SECIEventBus.h"
#include "Runtime/RHI/Public/RHIDevice.h"
#include "Runtime/RHI/Public/RHIBuffer.h"
#include "Public/Camera.h"
#include "Core/Logging/Public/Log.h"

namespace Sora
{

	class RenderingSubsystem : public SECI::SECISubsystem {
	public:
		explicit RenderingSubsystem(
			SECI::SECIEventBus* InEventBus,
			std::shared_ptr<RHI::RHIDevice> InRhiDevice,
			std::unique_ptr<Rendering::Camera> InCamera = nullptr
		) : EventBus(InEventBus), RhiDevice(InRhiDevice), ActiveCamera(std::move(InCamera)) {}

		virtual ~RenderingSubsystem() override = default;

		inline virtual bool OnInitialize() override {
			if (!RhiDevice) {
				SORA_ENGINE_ERROR("RenderingSubsystem, Failed to locate operational RHIDevice!");
				return false;
			}

			if (!ActiveCamera) {
				SORA_ENGINE_WARN("RenderingSubsystem, No camera provided. Creating a default Camera.");
				Rendering::CameraDesc CameraDesc{};
				ActiveCamera = std::make_unique<Rendering::Camera>(CameraDesc);
			}

			RHI::RHIBufferDesc UboDesc{};
			UboDesc.Size = sizeof(UniformBufferObject);
			UboDesc.Usage = RHI::RHIBufferUsage::UniformBuffer;
			UboDesc.CpuVisible = true;

			CameraUboBuffer = RhiDevice->CreateBuffer(UboDesc);
			if (!CameraUboBuffer) {
				SORA_ENGINE_ERROR("RenderingSubsystem: Failed to create Camera UBO!");
				return false;
			}

			// create global pipelines, render passes, and default material descriptors here

			SORA_ENGINE_INFO("RenderSubsystem Initialized successfully.");
			return true;
		}

		inline virtual void OnShutdown() override {
			if (RhiDevice) RhiDevice->WaitIdle();
			CameraUboBuffer.reset();
			ActiveCamera.reset();
		}

		inline virtual void OnUpdate(float deltaTime) override {
			if (!RhiDevice || !ActiveCamera || !CameraUboBuffer) return;

			ActiveCamera->UpdateUBO();

			void* Mapped = CameraUboBuffer->Map();
            if (Mapped) {
                std::memcpy(Mapped, &ActiveCamera->GetUBO(), sizeof(UniformBufferObject));
            }
		}

		Rendering::Camera* GetActiveCamera() const { return ActiveCamera.get(); }
        std::shared_ptr<RHI::RHIBuffer> GetCameraUBO() const { return CameraUboBuffer; }

		virtual std::string GetName() const override { return "RenderingSubsystem"; }

	private:
		SECI::SECIEventBus* EventBus{ nullptr };
		std::shared_ptr<RHI::RHIDevice> RhiDevice{ nullptr };

		std::unique_ptr<Rendering::Camera> ActiveCamera{ nullptr };
		std::shared_ptr<RHI::RHIBuffer> CameraUboBuffer{ nullptr };
	};

}
