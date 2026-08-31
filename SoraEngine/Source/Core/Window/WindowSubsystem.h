#pragma once

#include "Core/SECI/Subsystem/Public/SECISubsystem.h"
#include "Core/SECI/EventLayer/Public/SECIEventBus.h"
#include "Core/SECI/EventLayer/Public/SECIEventHandler.h"
#include "Core/SECI/EventLayer/Public/Events/WindowEvent.h"
#include "Core/Logging/Public/Log.h"
#include "Public/Window.h"

namespace Sora
{

	class SORA_API WindowSubsystem : public SECI::SECISubsystem {
	public:
		explicit WindowSubsystem(SECI::SECIEventBus* InEventBus, const WindowProperties InProperties = WindowProperties())
			: EventBus(InEventBus), Properties(InProperties)
		{}

		virtual ~WindowSubsystem() override = default;
		
		virtual bool OnInitialize() override {
			NativeWindow = Window::Create(Properties);
			if (!NativeWindow) {
                SORA_ENGINE_ERROR("Failed to initialize Native Window inside WindowSubsystem!");
                return false;
            }

			NativeWindow->SetEventCallback([this](SECI::SECIEvent& InEvent) {
				if (EventBus) EventBus->PostImmediate(InEvent);
			});

			SORA_ENGINE_INFO("WindowSubsystem initialized successfully.");
			return true;
		}

		virtual void OnUpdate(float deltaTime) override {
			if (NativeWindow) NativeWindow->OnUpdate();
		}

		virtual void OnShutdown() override {
			NativeWindow.reset();
			SORA_ENGINE_INFO("WindowSubsystem shut down.");
		}

		virtual void OnEvent(SECI::SECIEvent& InEvent) override {
			SECI::SECIEventHandler Dispatcher(InEvent);

			// Handle Window Resize
			Dispatcher.Dispatch<SECI::WindowResizeEvent>([this](SECI::WindowResizeEvent& e) {
				SORA_ENGINE_TRACE("Window Resized: {0}x{1}", e.GetWidth(), e.GetHeight());
				// Update internal cached dimensions or trigger graphics viewport updates if needed
				return false; // Return false to let downstream layers (like UI/Renderers) also receive this event
			});

			// Handle Window Close
			Dispatcher.Dispatch<SECI::WindowCloseEvent>([this](SECI::WindowCloseEvent& e) {
				SORA_ENGINE_INFO("Window close requested.");
				// Handle any pre-shutdown cleanup or signal application exit
				return false; 
			});

			// Handle Window Focus Lost / Gained
			Dispatcher.Dispatch<SECI::WindowLostFocusEvent>([this](SECI::WindowLostFocusEvent& e) {
				SORA_ENGINE_TRACE("Window lost focus.");
				return false;
			});
		}

		virtual SECI::SECIEventCategory GetSubscribedEventCategories() const override {
			return	SECI::EventCategoryInput |
					SECI::EventCategoryWindow;
        }

		virtual std::string GetName() const override { return "WindowsSubsytem"; }
		virtual SECI::SECISubsystemPriority GetPriority() const override { return SECI::SECISubsystemPriority::High; }
	
	private:
		SECI::SECIEventBus* EventBus{ nullptr };
		WindowProperties Properties;
		std::unique_ptr<Window> NativeWindow{ nullptr };
	};

} // namespace Sora
