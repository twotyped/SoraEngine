#include "Core/Application/Public/Application.h"
#include "Core/Logging/Public/Log.h"
#include "Core/SECI/EventLayer/Public/SECIEventHandler.h"
#include "Core/Window/WindowSubsystem.h"

namespace Sora
{

	Application::Application()
        : EventBus(std::make_unique<SECI::SECIEventBus>()),
          Registry(EventBus.get()) {}

    bool Application::Initialize() {
        // 1. Register Core Engine Subsystems
        AppSubsystem = Registry.RegisterSubsystem<ApplicationSubsystem>(EventBus.get());
        WinSubsystem = Registry.RegisterSubsystem<WindowSubsystem>(EventBus.get(), WindowProperties("SoraEngine: RuntimeSandbox", 1280, 720));

        // 2. Allow Sandbox / Editor to hook in custom subsystems (e.g., ImGuiSubsystem, EditorSubsystem, GameplaySubsystem)
        OnRegisterSubsystems(Registry);

        // 3. Initialize all systems in priority order
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
