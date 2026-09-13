#include "Core/Application/Public/Application.h"
#include "Core/Logging/Public/Log.h"
#include "Core/SECI/EventLayer/Public/SECIEventHandler.h"
#include "Core/Window/WindowSubsystem.h"
#include "Runtime/RHI/Public/Vulkan/VulkanDevice.h"

namespace Sora
{

	Application::Application()
        : EventBus(std::make_unique<SECI::SECIEventBus>()),
          Registry(EventBus.get()) {}

	bool Application::Initialize() {
		pAppSubsystem = Registry.RegisterSubsystem<ApplicationSubsystem>(EventBus.get());
		pWinSubsystem = Registry.RegisterSubsystem<WindowSubsystem>(EventBus.get(), WindowProperties("SoraEngine: RuntimeSandbox", 1280, 720));
		pVfsSubsystem = Registry.RegisterSubsystem<VFSSubsystem>();
		pSlangSubsystem = Registry.RegisterSubsystem<SlangSubsystem>(EventBus.get(), pVfsSubsystem, ShaderTargetAPI::SPIRV);
		pInputSubsystem = Registry.RegisterSubsystem<InputSubsystem>(EventBus.get());
		
		if (pWinSubsystem && !Registry.InitializeSubsystem(pWinSubsystem.get())) {
			SORA_ENGINE_ERROR("Failed to initialize WindowSubsystem!");
			return false;
		}

		void* NativeWindowHandle = pWinSubsystem->GetNativeWindowHandle();
		if (!NativeWindowHandle) {
			SORA_ENGINE_ERROR("WindowSubsystem failed to provide a valid NativeWindowHandle!");
			return false;
		}

		pRhiSubsystem = Registry.RegisterSubsystem<RHISubsystem>(EventBus.get(), std::make_shared<RHI::VulkanDevice>(), NativeWindowHandle);
		if (!pRhiSubsystem) {
			SORA_ENGINE_ERROR("Failed to register RHISubsystem!");
			return false;
		}

		Rendering::CameraDesc CameraDesc{};
		CameraDesc.FOV = 70;
		std::unique_ptr<Rendering::Camera> Camera = std::make_unique<Rendering::Camera>(CameraDesc);
		
		pRenderingSubsystem = Registry.RegisterSubsystem<RenderingSubsystem>(EventBus.get(), pRhiSubsystem->GetDevice(), std::move(Camera));

		OnRegisterSubsystems(Registry, pRhiSubsystem->GetDevice(), pSlangSubsystem, pRenderingSubsystem, pInputSubsystem, pWinSubsystem);

		return Registry.InitializeAll();
	}

    void Application::Run() {
        float deltaTime = 0.01f; // Placeholder delta calculation

        while (pAppSubsystem && pAppSubsystem->IsApplicationRunning()) {
            Registry.UpdateAll(deltaTime);
            EventBus->DispatchQueued();
        }

        Registry.ShutdownAll();
    }

} // namespace Sora
