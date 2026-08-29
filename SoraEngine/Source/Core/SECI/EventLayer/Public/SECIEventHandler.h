#pragma once

#include "Core/Core.h"
#include "SECIEvent.h"

#include <functional>

namespace Sora::SECI
{

	/**
     * @brief Dispatcher class that inspects a generic SECIEvent and routes it 
     * to a matching typed callback function if the event types align.
     */
    class SORA_API SECIEventHandler
    {
    public:
        explicit SECIEventHandler(SECIEvent& InEvent);

        /**
         * @brief Dispatches the event to a consumer callback if the event type matches T.
         * 
         * @tparam T The specific SECIEvent class (e.g., WindowResizeEvent).
         * @param func Callback function returning bool (true = handled, false = continue propagating).
         * @return true if the event matched T and was processed by the callback, false otherwise.
         */
		template<typename T, typename F>
		bool Dispatch(const F& func) {
			// Verify that T is derived from SECIEvent at compile time
			static_assert(std::is_base_of_v<SECIEvent, T>, "Template type T must derive from SECIEvent!");

			if (Event.GetEventType() == T::GetStaticType())
			{
				// If already handled by a higher-priority layer, stop dispatching
				if (Event.Handled)
				{
					return true;
				}

				// Cast to the concrete event type and invoke the callback
				Event.Handled |= func(static_cast<T&>(Event));
				return true;
			}

			return false;
		}

    private:
        SECIEvent& Event;
    };

} // namespace Sora::SECI
