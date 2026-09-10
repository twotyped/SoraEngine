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
#include "Runtime/Slang/SlangSubsystem.h"
#include "Core/VFS/VFSSubsystem.h"
#include "Runtime/Rendering/RenderingSubsystem.h"
#include "Core/Input/InputSubsystem.h"

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
		virtual void OnRegisterSubsystems(Sora::SECI::SECISubsystemRegistry& InRegistry, std::shared_ptr<Sora::RHI::RHIDevice> InDevice, std::shared_ptr<Sora::SlangSubsystem> InSlangSubsystem, std::shared_ptr<Sora::RenderingSubsystem> InRenderingSubsystem, std::shared_ptr<Sora::InputSubsystem> InInputSubsystem, std::shared_ptr<Sora::WindowSubsystem> InWindowSubsystem) {}

    protected:
        std::unique_ptr<SECI::SECIEventBus> EventBus;
        SECI::SECISubsystemRegistry Registry;

        std::shared_ptr<ApplicationSubsystem> pAppSubsystem;
        std::shared_ptr<WindowSubsystem> pWinSubsystem;
        std::shared_ptr<RHISubsystem> pRhiSubsystem;
        std::shared_ptr<VFSSubsystem> pVfsSubsystem;
        std::shared_ptr<SlangSubsystem> pSlangSubsystem;
        std::shared_ptr<RenderingSubsystem> pRenderingSubsystem;
        std::shared_ptr<InputSubsystem> pInputSubsystem;

	};

	Application* CreateApplication();
}
