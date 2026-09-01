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
			if (System->IsInitialized()) continue;
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
		for (auto It = RegisteredSubsystems.rbegin(); It != RegisteredSubsystems.rend(); ++It) {
			auto& System = *It;
			if (System->IsInitialized()) {
				System->OnShutdown();
				System->SetInitialized(false);
			}
		}
	}

} // namespace Sora::SECI