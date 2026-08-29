#include "Core/SECI/EventLayer/Public/SECIEventBus.h"

namespace Sora::SECI
{
	template<typename T>
	uint64_t SECIEventBus::Subscribe(SECIEventPriority InPriority, std::function<bool(T&)> InCallback) {
		uint16_t ID = NextSubscriberID++;
		Subscribers.push_back({
			ID,
			T::GetStaticType(),
			InPriority,
			[InCallback](SECIEvent& e) { return InCallback(static_cast<T&>(e)); }
			});

		std::stable_sort(Subscribers.begin(), Subscribers.end(),
			[](const SECISubscriber& a, const SECISubscriber& b) {
				return static_cast<uint8_t>(a.Priority) > static_cast<uint8_t>(b.Priority);
			});

		return ID;
	}

	void SECIEventBus::PostImmediate(SECIEvent& InEvent) {
		for (auto& subscriber : Subscribers)
		{
			if (InEvent.Handled) break;
			if (subscriber.TargetType == InEvent.GetEventType())
			{
				InEvent.Handled |= subscriber.Callback(InEvent);
			}
		}
	}

	template<typename T, typename... Args>
	void SECIEventBus::QueueEvent(Args&&... args) {
		T* EventMemory = Arena.Allocate<T>(std::forward<Args>(args)...);
		if (EventMemory)
		{
			EventQueue.push_back(EventMemory);
		}
	}

	void SECIEventBus::DispatchQueued() {
		// Sort queue by event priority before dispatch
		std::stable_sort(EventQueue.begin(), EventQueue.end(),
			[](const SECIEvent* a, const SECIEvent* b) {
				return static_cast<uint8_t>(a->GetPriority()) > static_cast<uint8_t>(b->GetPriority());
			});

		for (SECIEvent* Event : EventQueue)
		{
			PostImmediate(*Event);
		}

		EventQueue.clear();
		Arena.Reset(); // O(1) Memory Reset
	}

} // namespace Sora::SECI
