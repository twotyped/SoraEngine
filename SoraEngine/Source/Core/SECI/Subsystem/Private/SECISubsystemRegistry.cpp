#include "Core/SECI/Subsystem/Public/SECISubsystemRegistry.h"

namespace Sora::SECI
{

	void SECISubsystemRegistry::UnregisterSubsystem(SECISubsystem* InSubsystem) {
		if (EventBus && InSubsystem) {
			EventBus->Unsubscribe(InSubsystem);
		}
		
		RegisteredSubsystems.erase(
			std::remove_if(RegisteredSubsystems.begin(), RegisteredSubsystems.end(),
				[InSubsystem](const auto& ptr) { return ptr.get() == InSubsystem; }),
			RegisteredSubsystems.end());
	}

	bool SECISubsystemRegistry::InitializeAll() {
		for (auto& System : RegisteredSubsystems) {
			if (!System->OnInitialize()) {
				return false;
			}
		}
		return true;
	}

	void SECISubsystemRegistry::UpdateAll(float deltaTime) {
		for (auto& System : RegisteredSubsystems) {
			System->OnUpdate(deltaTime);
		}
	}

	void SECISubsystemRegistry::RouteEventAll(SECIEvent& InEvent) {
		for (auto& System : RegisteredSubsystems) {
			if (InEvent.Handled) break;
			System->OnEvent(InEvent);
		}
	}


	void SECISubsystemRegistry::ShutdownAll() {
		for (auto it = RegisteredSubsystems.rbegin(); it != RegisteredSubsystems.rend(); ++it) {
			if (EventBus) EventBus->Unsubscribe(it->get());
			(*it)->OnShutdown();
		}
		RegisteredSubsystems.clear();
	}

} // namespace Sora::SECI