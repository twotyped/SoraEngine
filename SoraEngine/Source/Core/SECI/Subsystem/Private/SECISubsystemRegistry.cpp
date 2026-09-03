#include "Core/SECI/Subsystem/Public/SECISubsystemRegistry.h"
#include "Core/Logging/Public/Log.h"

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

	bool SECISubsystemRegistry::InitializeSubsystem(SECISubsystem* InSubsystem) {
		if (!InSubsystem) return false;
		if (InSubsystem->IsInitialized()) {
			SORA_ENGINE_WARN("Subsystem '{0}' is already initialized.", InSubsystem->GetName());
			return true;
		}

		InSubsystem->SetInitialized(true);

		if (!InSubsystem->OnInitialize()) {
			InSubsystem->SetInitialized(false);
			return false;
		}

		return true;
	}

	bool SECISubsystemRegistry::InitializeAll() {
		for (auto& System : RegisteredSubsystems) {
			if (!System || System->IsInitialized()) continue;
			if (!InitializeSubsystem(System.get())) {
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

	void SECISubsystemRegistry::ShutdownSubsystem(SECISubsystem* InSubsystem) {
		if (!InSubsystem) return;
		if (InSubsystem->IsInitialized()) {
			InSubsystem->OnShutdown();
			InSubsystem->SetInitialized(false);
		} else {
			SORA_ENGINE_WARN("Subsystem '{0}' is not initialized or has already been shutdown.", InSubsystem->GetName());
		}
		UnregisterSubsystem(InSubsystem);
	}

	void SECISubsystemRegistry::ShutdownAll() {
		while (!RegisteredSubsystems.empty()) {
			ShutdownSubsystem(RegisteredSubsystems.back().get());
		}
	}

} // namespace Sora::SECI