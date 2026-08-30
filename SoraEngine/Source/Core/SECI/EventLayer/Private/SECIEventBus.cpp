#include "Core/SECI/EventLayer/Public/SECIEventBus.h"
#include "Core/Logging/Public/Log.h"

namespace Sora::SECI
{
	void SECIEventBus::PostImmediate(SECIEvent& InEvent) {
		uint32_t EventCategories = InEvent.GetCategoryFlags();
		SECIEventType EventType = InEvent.GetEventType();

		for (auto& Subscriber : Subscribers) {
			if (InEvent.Handled) return;

			bool TypeMatch = (Subscriber.TargetType != SECIEventType::None && Subscriber.TargetType == EventType);
			bool CategoryMatch = (Subscriber.CategoryMask != EventCategoryNone && (EventCategories & Subscriber.CategoryMask) != 0);


			if (TypeMatch || CategoryMatch) {
				if (TypeMatch == CategoryMatch) SORA_ENGINE_WARN("It is recommended to only subscribe to an EventType NOT included within your subscribed EventCategories.");
				InEvent.Handled |= Subscriber.Callback(InEvent);
			}
		}
	}

	void SECIEventBus::Unsubscribe(void* InOwner) {
		if (!InOwner) return;

		Subscribers.erase(
			std::remove_if(Subscribers.begin(), Subscribers.end(), [InOwner](const SECIEventSubscriber& sub) {
				return sub.Owner == InOwner;
			}),
			Subscribers.end()
		);
	}

	void SECIEventBus::DispatchQueued() {
		// Sort queue by event priority before dispatch
		std::stable_sort(EventQueue.begin(), EventQueue.end(),
			[](const SECIEvent* a, const SECIEvent* b) {
				return static_cast<uint8_t>(a->GetPriority()) > static_cast<uint8_t>(b->GetPriority());
			});

		for (SECIEvent* Event : EventQueue) {
			if (Event) PostImmediate(*Event);
		}

		EventQueue.clear();
		Arena.Reset(); // O(1) Memory Reset
	}

} // namespace Sora::SECI
