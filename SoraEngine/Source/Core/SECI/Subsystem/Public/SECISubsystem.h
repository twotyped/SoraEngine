#pragma once

#include "Core/SECI/EventLayer/Public/SECIEvent.h"
#include <string>

namespace Sora::SECI
{

	enum class SECISubsystemPriority {
		High	= 0,
		Normal	= 1,
		Low		= 2,
	};

	class SORA_API SECISubsystem {
	public:
		virtual ~SECISubsystem() = default;

		virtual bool OnInitialize() = 0;
		virtual void OnUpdate(float deltaTime) = 0;
		virtual void OnShutdown() = 0;

		virtual void OnEvent(SECIEvent& InEvent) {}

		virtual std::vector<SECIEventType> GetSubscribedEventTypes() const { return {}; }
		virtual SECIEventCategory GetSubscribedEventCategories() const { return EventCategoryNone; }

		virtual std::string GetName() const = 0;
		virtual SECISubsystemPriority GetPriority() const { return SECISubsystemPriority::Normal; }
	};

} // namespace Sora::SECI
