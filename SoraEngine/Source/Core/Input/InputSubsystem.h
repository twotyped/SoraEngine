#pragma once

#include "Core/SECI/Subsystem/Public/SECISubsystem.h"
#include "Core/SECI/EventLayer/Public/SECIEventBus.h"
#include "Core/SECI/EventLayer/Public/SECIEventHandler.h"
#include "Core/SECI/EventLayer/Public/Events/KeyEvent.h"
#include "Core/SECI/EventLayer/Public/Events/MouseEvent.h"
#include "Core/Logging/Public/Log.h"
#include "Public/KeyCodes.h"
#include "Public/MouseCodes.h"

namespace Sora
{

	class SORA_API InputSubsystem : public SECI::SECISubsystem {
	public:
		explicit InputSubsystem(SECI::SECIEventBus* InEventBus) :
			EventBus(InEventBus) {}

		virtual ~InputSubsystem() override = default;

		virtual bool OnInitialize() override {
			SORA_ENGINE_INFO("InputSubsystem initialized successfully.");
            return true;
		}

		virtual void OnUpdate(float deltaTime) override {}

		virtual void OnShutdown() override  {
            KeyStates.clear();
            MouseButtonStates.clear();
            ActionBindings.clear();
            SORA_ENGINE_INFO("InputSubsystem shut down.");
        }

		virtual void OnEvent(SECI::SECIEvent& InEvent) override  {
            SECI::SECIEventHandler Dispatcher(InEvent);

            // Handle Key Pressed
            Dispatcher.Dispatch<SECI::KeyPressedEvent>([this](SECI::KeyPressedEvent& e) {
                if (e.IsRepeat()) return true;
                
                KeyStates[e.GetKeyCode()] = true;
                return true;
            });

            // Handle Key Released
            Dispatcher.Dispatch<SECI::KeyReleasedEvent>([this](SECI::KeyReleasedEvent& e) {
                KeyStates[e.GetKeyCode()] = false;
                return true; 
            });

            // Handle Mouse Pressed
            Dispatcher.Dispatch<SECI::MouseButtonPressedEvent>([this](SECI::MouseButtonPressedEvent& e) {
                MouseButtonStates[e.GetMouseButton()] = true;
                return true;
            });

            // Handle Mouse Released
            Dispatcher.Dispatch<SECI::MouseButtonReleasedEvent>([this](SECI::MouseButtonReleasedEvent& e) {
                MouseButtonStates[e.GetMouseButton()] = false;
                return true;
            });

            Dispatcher.Dispatch<SECI::MouseMovedEvent>([this](SECI::MouseMovedEvent& e) {
				MouseDeltaX -= e.GetX();
                MouseDeltaY -= e.GetY();

				return false;
			});

            Dispatcher.Dispatch<SECI::WindowFocusEvent>([this](SECI::WindowFocusEvent& e) {
				MouseDeltaX = 0.0f;
				MouseDeltaY = 0.0f;
				return false;
			});
        }

		virtual SECI::SECIEventCategory GetSubscribedEventCategories() const override {
			return SECI::EventCategoryKeyboard | SECI::EventCategoryMouse | SECI::EventCategoryWindow;
		}

        virtual std::string GetName() const override { return "InputSubsystem"; }
        virtual SECI::SECISubsystemPriority GetPriority() const override { return SECI::SECISubsystemPriority::Normal; }

        void BindAction(const std::string& actionName, KeyCode key)  {
            ActionBindings[actionName] = key;
        }

        bool IsActionActive(const std::string& actionName) const  {
            auto it = ActionBindings.find(actionName);
            if (it != ActionBindings.end()) {
                return IsKeyActive(it->second);
            }
            return false;
        }

        bool IsKeyActive(KeyCode key) const  {
            auto it = KeyStates.find(key);
            return (it != KeyStates.end()) ? it->second : false;
        }

        bool IsMouseButtonActive(MouseCode button) const  {
            auto it = MouseButtonStates.find(button);
            return (it != MouseButtonStates.end()) ? it->second : false;
        }

        float GetMouseDeltaX() const { return MouseDeltaX; }
		float GetMouseDeltaY() const { return MouseDeltaY; }

        float GetAndResetMouseDeltaX() {
			float temp = MouseDeltaX;
			MouseDeltaX = 0.0f;
			return temp;
		}

		float GetAndResetMouseDeltaY() {
			float temp = MouseDeltaY;
			MouseDeltaY = 0.0f;
			return temp;
		}
	
	private:
		SECI::SECIEventBus* EventBus{ nullptr };

		std::unordered_map<KeyCode, bool> KeyStates;
		std::unordered_map<MouseCode, bool> MouseButtonStates;
        std::unordered_map<std::string, KeyCode> ActionBindings;

        float LastMouseX{ 0.0f };
		float LastMouseY{ 0.0f };
		float MouseDeltaX{ 0.0f };
		float MouseDeltaY{ 0.0f };
		bool FirstMouse{ true };
	};

} // namespace Sora
