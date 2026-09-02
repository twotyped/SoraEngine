#pragma once

#include "Core/Core.h"
#include "Core/Window/Public/Window.h"
#include "Core/SECI/EventLayer/Public/SECIEvent.h"
#include "Core/SECI/EventLayer/Public/SECIEventBus.h"
#include "Core/SECI/EventLayer/Public/Events/WindowEvent.h"
#include "Core/SECI/Subsystem/Public/SECISubsystemRegistry.h"
#include "Core/Application/ApplicationSubsystem.h"
#include "Runtime/RHI/RHISubsystem.h"
#include "Core/Window/WindowSubsystem.h"

#include <memory>

namespace Sora
{
	class SORA_API Application {
	public:
		Application();
		virtual ~Application() = default;

		Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

		/**
         * @brief Bootstraps core engine subsystems and calls client subsystem registration.
         */
        bool Initialize();

        /**
         * @brief Executes the primary engine loop.
         */
        void Run();

	protected:
        /**
         * @brief Override in Sandbox or Editor to register client-specific subsystems.
         */
		virtual void OnRegisterSubsystems(SECI::SECISubsystemRegistry& InRegistry, std::shared_ptr<RHI::RHIDevice> InDevice) {}

    protected:
        std::unique_ptr<SECI::SECIEventBus> EventBus;
        SECI::SECISubsystemRegistry Registry;

        std::shared_ptr<ApplicationSubsystem> AppSubsystem;
        std::shared_ptr<WindowSubsystem> WinSubsystem;
        std::shared_ptr<RHISubsystem> RhiSubsystem;

	};

	Application* CreateApplication();
}
