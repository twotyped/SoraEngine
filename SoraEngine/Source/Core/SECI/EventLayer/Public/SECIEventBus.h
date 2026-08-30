#pragma once

#include "Core/Core.h"
#include "SECIEvent.h"
#include "SECIEventArena.h"
#include "Core/SECI/Subsystem/Public/SECISubsystem.h"
#include <functional>
#include <vector>
#include <algorithm>

namespace Sora::SECI
{
    using SECIEventCallback = std::function<bool(SECIEvent&)>;

    struct SORA_API SECIEventSubscriber {
        uint64_t ID{ 0 };
        void* Owner{ nullptr };
        SECIEventType TargetType{ SECIEventType::None };
        SECIEventCategory CategoryMask{ EventCategoryNone };
        SECIEventPriority Priority{ SECIEventPriority::Normal };
        SECIEventCallback Callback;
    };

    class SORA_API SECIEventBus {
    public:
        SECIEventBus() = default;
        ~SECIEventBus() = default;

        SECIEventBus(const SECIEventBus&) = delete;
        SECIEventBus& operator=(const SECIEventBus&) = delete;

        static SECIEventBus& Get() {
            static SECIEventBus instance;
            return instance;
        }

		/**
		 * @brief Register a listener function for a specific event category mask.
		 */
		template<typename T = SECIEvent>
		uint64_t SubscribeToCategory(uint32_t InCategoryMask, SECIEventPriority InPriority, std::function<bool(T&)> InCallback, void* InOwner = nullptr) {
			uint64_t ID = NextSubscriberID++;

			SECIEventSubscriber sub;
			sub.ID = ID;
			sub.Owner = InOwner;
			sub.TargetType = SECIEventType::None;
			// Cast uint32_t mask to SECIEventCategory enum type
			sub.CategoryMask = static_cast<SECIEventCategory>(InCategoryMask); 
			sub.Priority = InPriority;
			sub.Callback = [InCallback](SECIEvent& e) { return InCallback(static_cast<T&>(e)); };

			Subscribers.push_back(sub);

			std::stable_sort(Subscribers.begin(), Subscribers.end(),
				[](const SECIEventSubscriber& a, const SECIEventSubscriber& b) {
					return static_cast<uint8_t>(a.Priority) > static_cast<uint8_t>(b.Priority);
				});

			return ID;
		}

        /**
         * @brief Register a listener function for a compile-time static event type (e.g. WindowCloseEvent).
         */
        template<typename T>
        uint64_t SubscribeToType(SECIEventPriority InPriority, std::function<bool(T&)> InCallback, void* InOwner = nullptr) {
            return SubscribeToDynamicType(T::GetStaticType(), InPriority, [InCallback](SECIEvent& e) { return InCallback(static_cast<T&>(e)); }, InOwner);
        }

        /**
         * @brief Register a listener function for a runtime SECIEventType enum value.
         */
        uint64_t SubscribeToDynamicType(SECIEventType InType, SECIEventPriority InPriority, SECIEventCallback InCallback, void* InOwner = nullptr) {
            uint64_t ID = NextSubscriberID++;

            SECIEventSubscriber sub;
            sub.ID = ID;
            sub.Owner = InOwner;
            sub.TargetType = InType;
            sub.CategoryMask = EventCategoryNone;
            sub.Priority = InPriority;
            sub.Callback = InCallback;

            Subscribers.push_back(sub);

            std::stable_sort(Subscribers.begin(), Subscribers.end(),
                [](const SECIEventSubscriber& a, const SECIEventSubscriber& b) {
                    return static_cast<uint8_t>(a.Priority) > static_cast<uint8_t>(b.Priority);
                });

            return ID;
        }

        /**
         * @brief Unsubscribes all listeners associated with a specific owner.
         */
        void Unsubscribe(void* Owner);

        /**
         * @brief Immediately dispatches an event synchronously down the subscriber chain.
         */
        void PostImmediate(SECIEvent& InEvent);

        /**
         * @brief Allocates event inside frame arena and queues it for deferred processing.
         */
        template<typename T, typename... Args>
        void QueueEvent(Args&&... args) {
            T* EventMemory = Arena.Allocate<T>(std::forward<Args>(args)...);
            if (EventMemory) {
                EventQueue.push_back(EventMemory);
            }
        }

        /**
         * @brief Processes all queued events in order of priority, then clears the frame memory.
         */
        void DispatchQueued();

    private:
        SECIEventArena Arena;
        std::vector<SECIEvent*> EventQueue;
        std::vector<SECIEventSubscriber> Subscribers;
        uint64_t NextSubscriberID{ 0 };
    };

} // namespace Sora::SECI
