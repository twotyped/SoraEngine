#pragma once

#include "Core/Core.h"
#include "SECIEvent.h"
#include "SECIEventArena.h"

#include <functional>
#include <vector>
#include <algorithm>

namespace Sora::SECI
{

	using SECIEventCallback = std::function<bool(SECIEvent&)>;

	struct SORA_API SECISubscriber
    {
        uint64_t ID;
        SECIEventType TargetType;
        SECIEventPriority Priority;
        SECIEventCallback Callback;
    };

    class SORA_API SECIEventBus
    {
    public:
        static SECIEventBus& Get()
        {
            static SECIEventBus instance;
            return instance;
        }

        /**
         * @brief Register a listener function for a specific event type.
         */
        template<typename T>
        uint64_t Subscribe(SECIEventPriority InPriority, std::function<bool(T&)> InCallback);

        /**
          * @brief Immediately dispatches an event synchronously down the subscriber chain.
          */
        void PostImmediate(SECIEvent& InEvent);

        /**
		 * @brief Allocates event inside frame arena and queues it for deferred processing.
		 */
        template<typename T, typename... Args>
        void QueueEvent(Args&&... args);

        /**
         * @brief Processes all queued events in order of priority, then clears the frame memory.
         */
        void DispatchQueued();

    private:
        SECIEventBus() = default;

        SECIEventArena Arena;
        std::vector<SECIEvent*> EventQueue;
        std::vector<SECISubscriber> Subscribers;
        uint16_t NextSubscriberID;
    };

} // namespace Sora::SECI