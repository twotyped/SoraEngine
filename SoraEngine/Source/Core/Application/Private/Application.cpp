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
		// 1. Register WindowSubsystem first
		AppSubsystem = Registry.RegisterSubsystem<ApplicationSubsystem>(EventBus.get());
		WinSubsystem = Registry.RegisterSubsystem<WindowSubsystem>(EventBus.get(), WindowProperties("SoraEngine: RuntimeSandbox", 1280, 720));

		// Initialize WindowSubsystem specifically so HWND is created on OS level
		if (WinSubsystem && !Registry.InitializeSubsystem(WinSubsystem.get())) {
			SORA_ENGINE_ERROR("Failed to initialize WindowSubsystem!");
			return false;
		}

		// 2. Fetch valid HWND
		void* NativeWindowHandle = WinSubsystem->GetNativeWindowHandle();
		if (!NativeWindowHandle) {
			SORA_ENGINE_ERROR("WindowSubsystem failed to provide a valid NativeWindowHandle!");
			return false;
		}

		// 3. Register RHI Subsystem
		RhiSubsystem = Registry.RegisterSubsystem<RHISubsystem>(EventBus.get(), std::make_shared<RHI::VulkanDevice>(), NativeWindowHandle);
		if (!RhiSubsystem) {
			SORA_ENGINE_ERROR("Failed to register RHISubsystem!");
			return false;
		}

		// 4. Register Sandbox / Client subsystems
		OnRegisterSubsystems(Registry, RhiSubsystem->GetDevice());

		// 5. Single unified initialization call for all remaining subsystems
		return Registry.InitializeAll();
	}

    void Application::Run() {
        float deltaTime = 0.01f; // Placeholder delta calculation

        while (AppSubsystem && AppSubsystem->IsApplicationRunning()) {
            Registry.UpdateAll(deltaTime);
            EventBus->DispatchQueued();
        }

        Registry.ShutdownAll();
    }

} // namespace Sora
