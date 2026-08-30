#pragma once

#include "SECISubsystem.h"
#include "Core/SECI/EventLayer/Public/SECIEventBus.h"
#include <vector>
#include <memory>
#include <algorithm>

namespace Sora::SECI
{

	class SORA_API SECISubsystemRegistry {
	private:
		std::vector<std::shared_ptr<SECISubsystem>> RegisteredSubsystems;
		SECIEventBus* EventBus{ nullptr };

	public:
		/**
         * @brief Constructs the registry with an optional event bus target for subscription auto-binding.
         * @param InEventBus Pointer to the SECIEventBus instance to route events through.
         */
		explicit SECISubsystemRegistry(SECIEventBus* InEventBus) : EventBus(InEventBus) {}

		/**
         * @brief Instantiates, sorts, and auto-subscribes a new subsystem.
         * @tparam T Concrete class derived from SECISubsystem.
         * @param args Constructor arguments forwarded to T.
         * @return std::shared_ptr<T> Pointer to the created subsystem.
         */
		template<typename T, typename... Args>
		std::shared_ptr<T> RegisterSubsystem(Args&&... args) {
			auto Subsystem = std::make_shared<T>(std::forward<Args>(args)...);
			RegisteredSubsystems.push_back(Subsystem);

			// 1. Sort subsystems based on priority
			std::sort(RegisteredSubsystems.begin(), RegisteredSubsystems.end(),
				[](const auto& a, const auto& b) {
					return static_cast<int>(a->GetPriority()) < static_cast<int>(b->GetPriority());
				});

			// 2. Auto-subscribe subsystem into EventBus
			if (EventBus) {
				// A. Dynamic type subscriptions (runtime enum IDs)
				for (SECIEventType Type : Subsystem->GetSubscribedEventTypes()) {
					EventBus->SubscribeToDynamicType(
						Type,
						SECIEventPriority::Normal,
						[subPtr = Subsystem.get()](SECIEvent& ev) {
							subPtr->OnEvent(ev);
							return ev.Handled;
						},
						Subsystem.get()
					);
				}

				// B. Category mask subscriptions
				SECIEventCategory CategoryMask = Subsystem->GetSubscribedEventCategories();
				if (CategoryMask != EventCategoryNone) {
					EventBus->SubscribeToCategory<SECIEvent>(
						CategoryMask,
						SECIEventPriority::Normal,
						[subPtr = Subsystem.get()](SECIEvent& ev) {
							subPtr->OnEvent(ev);
							return ev.Handled;
						},
						Subsystem.get()
					);
				}
			}

			return Subsystem;
		}

        /**
		 * @brief Unbinds a subsystem from the Event Bus and removes it from active registry management.
		 * @param InSubsystem Pointer to the subsystem instance to remove.
		 */
        void UnregisterSubsystem(SECISubsystem* InSubsystem);

        /**
         * @brief Calls OnInitialize() on all registered subsystems in order of priority.
         * @return true if all subsystems initialized successfully, false otherwise.
         */
        bool InitializeAll();

		/**
         * @brief Ticks all subsystems sequentially each frame (OS message pumping, physics updates, logic).
         */
        void UpdateAll(float deltaTime);
		/**
         * @brief Direct broadcast fallback if bypassing EventBus to send a raw SECIEvent to all systems.
         */
		void RouteEventAll(SECIEvent& InEvent);

        /**
         * @brief Unsubscribes and gracefully shuts down all registered subsystems in reverse priority order.
         */
        void ShutdownAll();
    };

} // namespace Sora::SECI
