#pragma once

#include "Core/SECI/Subsystem/Public/SECISubsystem.h"
#include "Core/SECI/EventLayer/Public/SECIEventBus.h"
#include "Core/SECI/EventLayer/Public/SECIEventHandler.h"
#include "Core/SECI/EventLayer/Public/Events/WindowEvent.h"
#include "Core/Logging/Public/Log.h"

namespace Sora
{

	class SORA_API ApplicationSubsystem : public SECI::SECISubsystem {
	public:
		explicit ApplicationSubsystem(SECI::SECIEventBus* InEventBus)
			: EventBus(InEventBus) {}

		virtual ~ApplicationSubsystem() override = default;

        virtual bool OnInitialize() override {
            SORA_ENGINE_INFO("ApplicationSubsystem Initialized.");
            return true;
        }

		virtual void OnUpdate(float deltaTime) override {
            // High-level frame logic or game loop tick coordination goes here
        }

		virtual void OnShutdown() override {
			SORA_ENGINE_INFO("ApplicationSubsystem Shutdown.");
		}

		virtual void OnEvent(SECI::SECIEvent& InEvent) override {
#ifdef SORA_DEBUG
            SORA_ENGINE_TRACE("ApplicationSubsystem, Event Received: {0}", InEvent.ToString());
#endif

            SECI::SECIEventHandler Dispatcher(InEvent);

            Dispatcher.Dispatch<SECI::WindowCloseEvent>([this](SECI::WindowCloseEvent& e) {
                SORA_ENGINE_INFO("Application received WindowCloseEvent. Requesting exit...");
                IsRunning = false;
                return true;
            });
        }

        virtual SECI::SECIEventCategory GetSubscribedEventCategories() const override {
            return SECI::EventCategoryWindow |
				   SECI::EventCategoryInput;
                   //SECI::EventCategoryMouse |
                   //SECI::EventCategoryMouseButton |
                   //SECI::EventCategoryKeyboard;
        }

        virtual std::string GetName() const override { return "ApplicationSubsystem"; }
        virtual SECI::SECISubsystemPriority GetPriority() const override { return SECI::SECISubsystemPriority::Low; }

        SORA_NO_DISCARD bool IsApplicationRunning() const { return IsRunning; }
        void RequestExit() { IsRunning = false; }

	private:
        SECI::SECIEventBus* EventBus{ nullptr };
        bool IsRunning = true;
	};

}
