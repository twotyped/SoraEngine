#include "Core/SECI/EventLayer/Public/SECIEvent.h"

namespace Sora::SECI
{

	SECIEvent::SECIEvent(SECIEventPriority InPriority)
		: Priority(InPriority)
	{
	}

	SECIEventPriority SECIEvent::GetPriority() const
	{
		return Priority;
	}

	std::string SECIEvent::ToString() const
	{
		return GetName();
	}

	bool SECIEvent::IsInCategory(SECIEventCategory InCategory) const
	{
		return (GetCategoryFlags() & static_cast<uint32_t>(InCategory)) != 0;
	}

} // namespace Sora::SECI